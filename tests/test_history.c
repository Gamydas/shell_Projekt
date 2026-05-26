/*
 Unit tests for src/history.c
 These tests were written with the help of claude code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../lib/history.h"
#include "../lib/str.h"
#include "../lib/err.h"

/* Access the globals defined in history.c */
extern shHist *first_entry;
extern shHist *last_entry;

/* ── simple test framework ─────────────────────────────────────────────── */

static int          tests_run    = 0;
static int          tests_passed = 0;
static int          tests_failed = 0;
static const char  *_current_test = "(unknown)";

#define MAX_FAILURES 64
static const char  *_failed_tests[MAX_FAILURES];
static int          _failed_count = 0;

static void _record_failure(void)
{
    for (int i = 0; i < _failed_count; i++)
        if (_failed_tests[i] == _current_test) return;
    if (_failed_count < MAX_FAILURES)
        _failed_tests[_failed_count++] = _current_test;
}

#define ASSERT(cond, msg)                                                                    \
    do {                                                                                     \
        tests_run++;                                                                         \
        if (cond) {                                                                          \
            printf("  [PASS] %s\n", (msg));                                                 \
            tests_passed++;                                                                  \
        } else {                                                                             \
            printf("  [FAIL] %s\n         ^-- test: %s  line %d\n", (msg), _current_test, __LINE__); \
            tests_failed++;                                                                  \
            _record_failure();                                                               \
        }                                                                                    \
    } while (0)

#define ASSERT_EQ(a, b, msg)    ASSERT((a) == (b),         (msg))
#define ASSERT_NE(a, b, msg)    ASSERT((a) != (b),         (msg))
#define ASSERT_NULL(p, msg)     ASSERT((p) == NULL,        (msg))
#define ASSERT_NOT_NULL(p, msg) ASSERT((p) != NULL,        (msg))
#define ASSERT_STR_EQ(a, b, msg) ASSERT(strcmp((a),(b))==0,(msg))

#define RUN_TEST(fn)                                                       \
    do {                                                                   \
        _current_test = #fn;                                               \
        printf("\n[TEST] %s\n", #fn);                                     \
        fn();                                                              \
        teardown();                                                        \
    } while (0)

/* ── test helpers ──────────────────────────────────────────────────────── */

/*
 * Walk and free the entire list, then reset globals.
 * Also handles the case where write_history_to_file has walked first_entry
 * to NULL but left last_entry set — in that case we walk backward from
 * last_entry so the nodes are still freed.
 */
static void teardown(void)
{
    if (first_entry == NULL && last_entry != NULL) {
        /* write_history_to_file consumed first_entry; walk backward */
        shHist *curr = last_entry;
        while (curr != NULL) {
            shHist *prev = curr->prev;
            free(curr->entry);
            free(curr);
            curr = prev;
        }
    } else {
        shHist *curr = first_entry;
        while (curr != NULL) {
            shHist *next = curr->next;
            free(curr->entry);
            free(curr);
            curr = next;
        }
    }
    first_entry = NULL;
    last_entry  = NULL;
}

/*
 * Manually build the very first history entry and wire the globals.
 * Needed because create_and_append_new_hist_entry requires last_entry != NULL
 * (BUG-1).
 */
static shHist *setup_first_entry(const char *text, uint16_t id)
{
    shHist *e = malloc(sizeof(shHist));
    if (!e) return NULL;
    size_t len = str_len(text);
    e->entry = malloc(len + 1);
    if (!e->entry) { free(e); return NULL; }
    memcpy(e->entry, text, len + 1);
    e->entry_size = (uint16_t)len;
    e->entry_ID   = id;
    e->next       = NULL;
    e->prev       = NULL;
    first_entry   = e;
    last_entry    = e;
    return e;
}

/* ── stdout capture helpers ────────────────────────────────────────────── */

static int  _saved_stdout_fd = -1;
static char _capture_path[64];

static void start_stdout_capture(void)
{
    /* Flush pending buffered output before redirecting fd 1.
     * Without this, fully-buffered stdout (e.g. when piped through make)
     * would send buffered [PASS] text from prior tests into the capture. */
    fflush(stdout);
    strcpy(_capture_path, "/tmp/hist_test_out_XXXXXX");
    int fd = mkstemp(_capture_path);
    _saved_stdout_fd = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    close(fd);
}

static char *end_stdout_capture(void)
{
    static char buf[4096];
    fflush(stdout);
    dup2(_saved_stdout_fd, STDOUT_FILENO);
    close(_saved_stdout_fd);
    _saved_stdout_fd = -1;

    FILE *f = fopen(_capture_path, "r");
    size_t n = 0;
    if (f) {
        n = fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
    }
    buf[n] = '\0';
    unlink(_capture_path);
    return buf;
}

/* ── temp HOME helpers for file-based tests ────────────────────────────── */

static char _temp_home[64];
static char _saved_home[1024];
static char _saved_cwd[1024];

static void setup_temp_home(void)
{
    getcwd(_saved_cwd, sizeof(_saved_cwd));
    const char *home = getenv("HOME");
    if (home)
        strncpy(_saved_home, home, sizeof(_saved_home) - 1);
    else
        _saved_home[0] = '\0';

    strcpy(_temp_home, "/tmp/hist_test_home_XXXXXX");
    mkdtemp(_temp_home);
    setenv("HOME", _temp_home, 1);
}

static void cleanup_temp_home(void)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/.myshellhistory", _temp_home);
    unlink(path);
    rmdir(_temp_home);

    if (_saved_home[0])
        setenv("HOME", _saved_home, 1);
    else
        unsetenv("HOME");

    chdir(_saved_cwd);
}

/* Write content directly into .myshellhistory in the temp HOME. */
static void write_hist_file(const char *content)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/.myshellhistory", _temp_home);
    FILE *f = fopen(path, "w");
    if (f) { fputs(content, f); fclose(f); }
}

/* Read .myshellhistory from temp HOME into buf. Returns byte count or -1. */
static int read_hist_file(char *buf, size_t size)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/.myshellhistory", _temp_home);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    size_t n = fread(buf, 1, size - 1, f);
    buf[n] = '\0';
    fclose(f);
    return (int)n;
}

/* ══════════════════════════════════════════════════════════════════════════
 * create_and_append_new_hist_entry
 * ══════════════════════════════════════════════════════════════════════════ */

static void test_append_returns_zero_on_success(void)
{
    setup_first_entry("init", 1);
    int ret = create_and_append_new_hist_entry("ls", 2);
    ASSERT_EQ(ret, 0, "returns 0 on success");
}

static void test_append_updates_last_entry(void)
{
    setup_first_entry("init", 1);
    create_and_append_new_hist_entry("pwd", 3);
    ASSERT_NOT_NULL(last_entry, "last_entry is not NULL after append");
    ASSERT_STR_EQ(last_entry->entry, "pwd", "last_entry->entry matches appended string");
}

static void test_append_entry_size_stored(void)
{
    setup_first_entry("init", 1);
    create_and_append_new_hist_entry("echo", 4);
    ASSERT_EQ(last_entry->entry_size, 4, "entry_size stored correctly");
}

static void test_append_id_is_predecessor_plus_one(void)
{
    setup_first_entry("cmd0", 5);
    create_and_append_new_hist_entry("cmd1", 4);
    ASSERT_EQ(last_entry->entry_ID, 6, "new entry ID = previous ID + 1");
}

static void test_append_forward_link(void)
{
    shHist *first = setup_first_entry("first", 1);
    create_and_append_new_hist_entry("second", 6);
    ASSERT_NOT_NULL(first->next, "first->next set after append");
    ASSERT_STR_EQ(first->next->entry, "second", "first->next points to new entry");
}

static void test_append_backward_link(void)
{
    shHist *first = setup_first_entry("first", 1);
    create_and_append_new_hist_entry("second", 6);
    ASSERT_EQ(last_entry->prev, first, "new entry->prev points to previous last");
}

static void test_append_last_entry_next_is_null(void)
{
    setup_first_entry("init", 1);
    create_and_append_new_hist_entry("tail", 4);
    ASSERT_NULL(last_entry->next, "last_entry->next is NULL after append");
}

static void test_append_multiple_ids_sequential(void)
{
    setup_first_entry("a", 1);
    create_and_append_new_hist_entry("b", 1);
    create_and_append_new_hist_entry("c", 1);
    create_and_append_new_hist_entry("d", 1);

    shHist *curr = first_entry;
    uint16_t expected = 1;
    int ok = 1;
    while (curr) {
        if (curr->entry_ID != expected) { ok = 0; break; }
        expected++;
        curr = curr->next;
    }
    ASSERT(ok, "IDs are sequential across all appended entries");
}

static void test_append_preserves_full_string(void)
{
    const char *cmd = "grep -rn \"hello world\" /path/to/src --include='*.c'";
    setup_first_entry("init", 1);
    create_and_append_new_hist_entry((char *)cmd, (uint16_t)str_len(cmd));
    ASSERT_STR_EQ(last_entry->entry, cmd, "complex command string preserved exactly");
}

static void test_append_list_length_grows(void)
{
    setup_first_entry("0", 1);
    create_and_append_new_hist_entry("1", 1);
    create_and_append_new_hist_entry("2", 1);
    create_and_append_new_hist_entry("3", 1);

    int count = 0;
    for (shHist *c = first_entry; c; c = c->next) count++;
    ASSERT_EQ(count, 4, "list grows to 4 after 3 appends to a 1-entry list");
}

static void test_append_bidirectional_traversal_consistent(void)
{
    setup_first_entry("x", 1);
    create_and_append_new_hist_entry("y", 1);
    create_and_append_new_hist_entry("z", 1);

    /* Walk forward to the end */
    shHist *fwd = first_entry;
    while (fwd->next) fwd = fwd->next;
    ASSERT_EQ(fwd, last_entry, "forward traversal ends at last_entry");

    /* Walk backward to the start */
    shHist *bwd = last_entry;
    while (bwd->prev) bwd = bwd->prev;
    ASSERT_EQ(bwd, first_entry, "backward traversal ends at first_entry");
}

/* ══════════════════════════════════════════════════════════════════════════
 * find_in_history
 * ══════════════════════════════════════════════════════════════════════════ */

static void test_find_single_entry(void)
{
    setup_first_entry("only", 1);
    shHist *found = find_in_history(1);
    ASSERT_NOT_NULL(found, "can find the only entry in a 1-element list");
    ASSERT_EQ(found->entry_ID, 1, "found entry has the correct ID");
}

static void test_find_last_entry(void)
{
    setup_first_entry("first", 1);
    create_and_append_new_hist_entry("second", 6);
    create_and_append_new_hist_entry("third",  5);

    uint16_t last_id = last_entry->entry_ID;
    shHist *found = find_in_history(last_id);
    ASSERT_NOT_NULL(found, "can find last entry by ID");
    ASSERT_EQ(found, last_entry, "found pointer equals last_entry");
}

static void test_find_middle_entry(void)
{
    setup_first_entry("a", 1);
    create_and_append_new_hist_entry("b", 1);
    create_and_append_new_hist_entry("c", 1);
    create_and_append_new_hist_entry("d", 1);
    create_and_append_new_hist_entry("e", 1);

    shHist *found = find_in_history(3);
    ASSERT_NOT_NULL(found, "can find middle entry (ID=3)");
    ASSERT_EQ(found->entry_ID, 3, "found entry has correct ID");
    ASSERT_STR_EQ(found->entry, "c", "found entry has correct content");
}

static void test_find_nonexistent_returns_null(void)
{
    setup_first_entry("only", 1);
    shHist *found = find_in_history(999);
    ASSERT_NULL(found, "find returns NULL for non-existent ID");
}

static void test_find_returns_correct_pointer(void)
{
    setup_first_entry("first", 1);
    create_and_append_new_hist_entry("second", 6);
    create_and_append_new_hist_entry("third",  5);

    shHist *found = find_in_history(2);
    ASSERT_NOT_NULL(found, "find returns non-NULL for ID=2");
    ASSERT_STR_EQ(found->entry, "second", "entry at ID=2 has correct content");
}

static void test_find_returns_null_on_empty_list(void)
{
    shHist *found = find_in_history(1);
    ASSERT_NULL(found, "find returns NULL on empty history");
}

/* ══════════════════════════════════════════════════════════════════════════
 * delete_from_history
 * ══════════════════════════════════════════════════════════════════════════ */

static void test_delete_middle_entry_not_findable(void)
{
    setup_first_entry("cmd1", 4);
    create_and_append_new_hist_entry("cmd2", 4);
    create_and_append_new_hist_entry("cmd3", 4);

    delete_from_history(2);

    shHist *found = find_in_history(2);
    ASSERT_NULL(found, "deleted entry (ID=2) can no longer be found");
}

static void test_delete_middle_reconnects_list(void)
{
    shHist *e1 = setup_first_entry("first", 1);
    create_and_append_new_hist_entry("middle", 6);
    create_and_append_new_hist_entry("last", 4);
    shHist *e3 = last_entry;

    delete_from_history(2);

    ASSERT_EQ(e1->next, e3, "after deleting middle: first->next points to last");
    ASSERT_EQ(e3->prev, e1, "after deleting middle: last->prev points to first");
}

static void test_delete_nonexistent_id_leaves_list_intact(void)
{
    setup_first_entry("only", 1);
    /* Should print an error message but must not crash. */
    delete_from_history(999);
    ASSERT_NOT_NULL(first_entry, "list intact after deleting non-existent ID");
    ASSERT_EQ(first_entry->entry_ID, 1, "remaining entry has correct ID");
}

static void test_delete_reduces_list_count(void)
{
    setup_first_entry("a", 1);
    create_and_append_new_hist_entry("b", 1);
    create_and_append_new_hist_entry("c", 1);
    create_and_append_new_hist_entry("d", 1);

    delete_from_history(2);
    delete_from_history(3);

    int count = 0;
    for (shHist *c = first_entry; c; c = c->next) count++;
    ASSERT_EQ(count, 2, "list has 2 entries after deleting 2 of 4");
}

static void test_delete_first_entry(void)
{
    setup_first_entry("first", 1);
    create_and_append_new_hist_entry("second", 6);
    delete_from_history(1);
    ASSERT_EQ(first_entry->entry_ID, 2, "first_entry updated after deleting old first");
    ASSERT_NULL(first_entry->prev, "new first_entry->prev is NULL");
}

static void test_delete_last_entry(void)
{
    setup_first_entry("first", 1);
    create_and_append_new_hist_entry("second", 6);
    delete_from_history(2);
    ASSERT_EQ(last_entry->entry_ID, 1, "last_entry updated after deleting old last");
    ASSERT_NULL(last_entry->next, "new last_entry->next is NULL");
}

/* ══════════════════════════════════════════════════════════════════════════
 * free_history_entry
 * ══════════════════════════════════════════════════════════════════════════ */

static void test_free_null_does_not_crash(void)
{
    free_history_entry(NULL);
    ASSERT(1, "free_history_entry(NULL) does not crash");
}

static void test_free_valid_detached_entry(void)
{
    shHist *e = malloc(sizeof(shHist));
    e->entry      = malloc(6);
    memcpy(e->entry, "hello", 6);
    e->entry_size = 5;
    e->entry_ID   = 99;
    e->next       = NULL;
    e->prev       = NULL;

    free_history_entry(e);
    ASSERT(1, "free_history_entry frees a valid detached entry without crashing");
}

/* ══════════════════════════════════════════════════════════════════════════
 * clear_shell_history
 * ══════════════════════════════════════════════════════════════════════════ */

static void test_clear_empty_history_does_not_crash(void)
{
    clear_shell_history();
    ASSERT(1, "clear_shell_history on empty list does not crash");
    ASSERT_NULL(first_entry, "first_entry remains NULL after clearing empty list");
    ASSERT_NULL(last_entry,  "last_entry remains NULL after clearing empty list");
}

static void test_clear_single_entry_sets_globals_null(void)
{
    setup_first_entry("cmd", 3);
    clear_shell_history();
    ASSERT_NULL(first_entry, "first_entry is NULL after clearing single-entry list");
    ASSERT_NULL(last_entry,  "last_entry is NULL after clearing single-entry list");
}

static void test_clear_multiple_entries_sets_globals_null(void)
{
    setup_first_entry("a", 1);
    create_and_append_new_hist_entry("b", 1);
    create_and_append_new_hist_entry("c", 1);
    clear_shell_history();
    ASSERT_NULL(first_entry, "first_entry is NULL after clearing multi-entry list");
    ASSERT_NULL(last_entry,  "last_entry is NULL after clearing multi-entry list");
}

static void test_clear_allows_append_after_clear(void)
{
    setup_first_entry("old1", 4);
    create_and_append_new_hist_entry("old2", 4);
    clear_shell_history();
    setup_first_entry("new_first", 9);
    int ret = create_and_append_new_hist_entry("new_second", 10);
    ASSERT_EQ(ret, 0, "create_and_append returns 0 after history is cleared");
    ASSERT_NOT_NULL(last_entry, "last_entry is set after appending to cleared history");
    ASSERT_STR_EQ(last_entry->entry, "new_second", "new entry content is correct after clear");
}

static void test_clear_idempotent_on_already_empty_list(void)
{
    clear_shell_history();
    clear_shell_history();
    ASSERT(1, "double clear on empty list does not crash");
    ASSERT_NULL(first_entry, "first_entry still NULL after double clear");
    ASSERT_NULL(last_entry,  "last_entry still NULL after double clear");
}

static void test_clear_list_is_not_traversable_after_clear(void)
{
    setup_first_entry("x", 1);
    create_and_append_new_hist_entry("y", 1);
    clear_shell_history();
    /* After a correct clear the list is empty; traversal must yield 0 entries. */
    int count = 0;
    for (shHist *c = first_entry; c; c = c->next) count++;
    ASSERT_EQ(count, 0, "traversal finds 0 entries after clear");
}

static void test_clear_find_returns_null_after_clear(void)
{
    setup_first_entry("only", 1);
    clear_shell_history();
    shHist *found = find_in_history(1);
    ASSERT_NULL(found, "find_in_history returns NULL for any ID after clear");
}

/* ══════════════════════════════════════════════════════════════════════════
 * print_history
 * ══════════════════════════════════════════════════════════════════════════ */

static void test_print_empty_history_produces_no_output(void)
{
    start_stdout_capture();
    print_history();
    char *out = end_stdout_capture();
    ASSERT_EQ((int)str_len(out), 0, "print_history on empty list produces no output");
}

static void test_print_single_entry_contains_id(void)
{
    setup_first_entry("ls", 1);
    start_stdout_capture();
    print_history();
    char *out = end_stdout_capture();
    ASSERT(strstr(out, "1") != NULL, "print_history output contains entry ID");
}

static void test_print_single_entry_contains_text(void)
{
    setup_first_entry("ls -la", 1);
    start_stdout_capture();
    print_history();
    char *out = end_stdout_capture();
    ASSERT(strstr(out, "ls -la") != NULL, "print_history output contains entry text");
}

static void test_print_multiple_entries_in_order(void)
{
    setup_first_entry("first_cmd", 1);
    create_and_append_new_hist_entry("second_cmd", 10);
    create_and_append_new_hist_entry("third_cmd", 9);

    start_stdout_capture();
    print_history();
    char *out = end_stdout_capture();

    char *pos_first  = strstr(out, "first_cmd");
    char *pos_second = strstr(out, "second_cmd");
    char *pos_third  = strstr(out, "third_cmd");

    ASSERT_NOT_NULL(pos_first,  "output contains first_cmd");
    ASSERT_NOT_NULL(pos_second, "output contains second_cmd");
    ASSERT_NOT_NULL(pos_third,  "output contains third_cmd");
    ASSERT(pos_first < pos_second, "first_cmd appears before second_cmd in output");
    ASSERT(pos_second < pos_third, "second_cmd appears before third_cmd in output");
}

static void test_print_id_appears_before_entry_text(void)
{
    /* Output format: "%d  %s\n" — ID precedes its text on the same line. */
    setup_first_entry("pwd", 1);
    start_stdout_capture();
    print_history();
    char *out = end_stdout_capture();

    char *pos_id  = strstr(out, "1");
    char *pos_txt = strstr(out, "pwd");
    ASSERT_NOT_NULL(pos_id,  "output contains ID 1");
    ASSERT_NOT_NULL(pos_txt, "output contains 'pwd'");
    ASSERT(pos_id < pos_txt, "ID appears before its entry text on the line");
}

static void test_print_each_entry_on_separate_line(void)
{
    setup_first_entry("alpha", 1);
    create_and_append_new_hist_entry("beta", 4);

    start_stdout_capture();
    print_history();
    char *out = end_stdout_capture();

    int newlines = 0;
    for (char *p = out; *p; p++)
        if (*p == '\n') newlines++;
    ASSERT_EQ(newlines, 2, "two entries produce exactly two newlines");
}

static void test_print_all_ids_present(void)
{
    setup_first_entry("cmd", 1);
    create_and_append_new_hist_entry("cmd2", 4);
    create_and_append_new_hist_entry("cmd3", 4);

    start_stdout_capture();
    print_history();
    char *out = end_stdout_capture();

    ASSERT(strstr(out, "1") != NULL, "ID 1 is present in output");
    ASSERT(strstr(out, "2") != NULL, "ID 2 is present in output");
    ASSERT(strstr(out, "3") != NULL, "ID 3 is present in output");
}

static void test_print_preserves_special_characters(void)
{
    const char *cmd = "grep -rn 'hello world' ./src";
    setup_first_entry((char *)cmd, (uint16_t)str_len(cmd));

    start_stdout_capture();
    print_history();
    char *out = end_stdout_capture();

    ASSERT(strstr(out, cmd) != NULL, "output preserves entry with spaces and quotes");
}

/* ══════════════════════════════════════════════════════════════════════════
 * write_history_to_file
 *
 * NOTE: write_history_to_file modifies the global first_entry by walking
 * the list through it, leaving first_entry == NULL after the call.
 * The teardown() helper handles this by walking backward from last_entry.
 *
 * NOTE: the function has no return statement for the success path, so the
 * return value is technically undefined behaviour.  In practice on x86-64
 * with GCC the last fclose(0) return value is left in rax.
 * ══════════════════════════════════════════════════════════════════════════ */

static void test_write_creates_history_file(void)
{
    setup_temp_home();
    setup_first_entry("ls", 1);
    write_history_to_file();

    char path[128];
    snprintf(path, sizeof(path), "%s/.myshellhistory", _temp_home);
    FILE *f = fopen(path, "r");
    ASSERT_NOT_NULL(f, "write_history_to_file creates .myshellhistory");
    if (f) fclose(f);

    cleanup_temp_home();
}

static void test_write_empty_history_creates_empty_file(void)
{
    setup_temp_home();
    /* Empty list: the write loop doesn't execute; file should be empty. */
    write_history_to_file();

    char buf[256] = {0};
    int n = read_hist_file(buf, sizeof(buf));
    ASSERT_EQ(n, 0, "file is empty when history was empty");

    cleanup_temp_home();
}

static void test_write_single_entry_file_content(void)
{
    setup_temp_home();
    setup_first_entry("echo hello", 1);
    write_history_to_file();

    char buf[256];
    read_hist_file(buf, sizeof(buf));
    ASSERT(strstr(buf, "echo hello") != NULL, "file contains the written entry text");

    cleanup_temp_home();
}

static void test_write_multiple_entries_file_content(void)
{
    setup_temp_home();
    setup_first_entry("cmd1", 1);
    create_and_append_new_hist_entry("cmd2", 4);
    create_and_append_new_hist_entry("cmd3", 4);
    write_history_to_file();

    char buf[512];
    read_hist_file(buf, sizeof(buf));

    char *p1 = strstr(buf, "cmd1");
    char *p2 = strstr(buf, "cmd2");
    char *p3 = strstr(buf, "cmd3");
    ASSERT_NOT_NULL(p1, "file contains cmd1");
    ASSERT_NOT_NULL(p2, "file contains cmd2");
    ASSERT_NOT_NULL(p3, "file contains cmd3");
    ASSERT(p1 < p2, "cmd1 appears before cmd2 in file");
    ASSERT(p2 < p3, "cmd2 appears before cmd3 in file");

    cleanup_temp_home();
}

static void test_write_each_entry_on_separate_line(void)
{
    setup_temp_home();
    setup_first_entry("line1", 1);
    create_and_append_new_hist_entry("line2", 5);
    write_history_to_file();

    char buf[256];
    read_hist_file(buf, sizeof(buf));

    int newlines = 0;
    for (char *p = buf; *p; p++)
        if (*p == '\n') newlines++;
    ASSERT_EQ(newlines, 2, "two entries produce two newlines in file");

    cleanup_temp_home();
}

static void test_write_overwrites_existing_file(void)
{
    setup_temp_home();
    write_hist_file("old stale content\n");
    setup_first_entry("fresh_cmd", 9);
    write_history_to_file();

    char buf[256];
    read_hist_file(buf, sizeof(buf));
    ASSERT(strstr(buf, "old stale content") == NULL, "old file content is overwritten");
    ASSERT(strstr(buf, "fresh_cmd")         != NULL, "new entry present after overwrite");

    cleanup_temp_home();
}

static void test_write_leaves_first_entry_null(void)
{
    /* write_history_to_file walks the list via first_entry; afterwards
     * first_entry is NULL.  This is a known side-effect / bug. */
    setup_temp_home();
    setup_first_entry("a", 1);
    create_and_append_new_hist_entry("b", 1);
    write_history_to_file();
    ASSERT_NULL(first_entry, "first_entry is NULL after write (side-effect of walking the list)");
    cleanup_temp_home();
}

/* ══════════════════════════════════════════════════════════════════════════
 * read_history_from_file
 *
 * BUG NOTE: create_and_append_new_hist_entry crashes when called on an
 * empty list — the last_entry == NULL branch assigns new = first_entry
 * (NULL) then immediately dereferences it (new->entry_ID = 1).
 * read_history_from_file will therefore segfault the first time it tries
 * to add an entry into an empty list.  Tests that trigger this path are
 * placed last so all preceding tests still produce output.
 *
 * BUG NOTE: read_history_from_file calls clear_shell_history() when the
 * list is non-empty before reading.  clear_shell_history frees all nodes
 * but leaves first_entry/last_entry as dangling pointers.  Subsequent
 * create_and_append calls then use freed memory (use-after-free).
 * ══════════════════════════════════════════════════════════════════════════ */

static void test_read_missing_file_returns_minus_one(void)
{
    setup_temp_home();
    int ret = read_history_from_file();
    ASSERT_EQ(ret, -1, "read_history_from_file returns -1 when file does not exist");
    cleanup_temp_home();
}

static void test_read_does_not_disturb_existing_history_on_failure(void)
{
    setup_temp_home();
    read_history_from_file();
    ASSERT_NULL(first_entry, "first_entry remains NULL after failed read");
    ASSERT_NULL(last_entry,  "last_entry remains NULL after failed read");
    cleanup_temp_home();
}

static void test_read_empty_file_results_in_empty_history(void)
{
    /* Empty file: fgets immediately returns NULL, loop never runs,
     * create_and_append is never called — this path is safe. */
    setup_temp_home();
    write_hist_file("");
    read_history_from_file();
    ASSERT_NULL(first_entry, "first_entry is NULL after reading an empty file");
    ASSERT_NULL(last_entry,  "last_entry is NULL after reading an empty file");
    cleanup_temp_home();
}

static void test_read_populates_history_from_file(void)
{
    /* NOTE: segfaults at runtime — create_and_append_new_hist_entry
     * dereferences NULL in the empty-list branch (new = NULL, new->entry_ID). */
    setup_temp_home();
    write_hist_file("ls\npwd\n");
    read_history_from_file();
    ASSERT_NOT_NULL(first_entry, "first_entry is non-NULL after reading 2-line file");
    ASSERT_NOT_NULL(last_entry,  "last_entry is non-NULL after reading 2-line file");
    cleanup_temp_home();
    first_entry = NULL;
    last_entry  = NULL;
}

static void test_read_correct_entry_count(void)
{
    /* NOTE: segfaults at runtime — same create_and_append empty-list bug. */
    setup_temp_home();
    write_hist_file("cmd1\ncmd2\ncmd3\n");
    read_history_from_file();
    int count = 0;
    for (shHist *c = first_entry; c; c = c->next) count++;
    ASSERT_EQ(count, 3, "history has 3 entries after reading a 3-line file");
    cleanup_temp_home();
    first_entry = NULL;
    last_entry  = NULL;
}

static void test_read_clears_existing_history_first(void)
{
    /* NOTE: use-after-free — clear_shell_history frees nodes but leaves
     * first_entry/last_entry dangling; create_and_append then reads the
     * freed last_entry to compute the new entry's ID. */
    setup_temp_home();
    setup_first_entry("old_entry", 9);
    write_hist_file("new_entry\n");
    read_history_from_file();
    shHist *found = find_in_history(1);
    ASSERT(found == NULL || strcmp(found->entry, "old_entry") != 0,
           "old entry is gone after re-reading history from file");
    cleanup_temp_home();
    first_entry = NULL;
    last_entry  = NULL;
}

/* ══════════════════════════════════════════════════════════════════════════
 * main
 * ══════════════════════════════════════════════════════════════════════════ */

int main(void)
{
    /* Disable buffering so printf output is visible even when the process
     * aborts mid-run due to a crash triggered by a buggy function. */
    setbuf(stdout, NULL);

    printf("=== History Module Unit Tests ===\n");

    printf("\n-- create_and_append_new_hist_entry --\n");
    RUN_TEST(test_append_returns_zero_on_success);
    RUN_TEST(test_append_updates_last_entry);
    RUN_TEST(test_append_entry_size_stored);
    RUN_TEST(test_append_id_is_predecessor_plus_one);
    RUN_TEST(test_append_forward_link);
    RUN_TEST(test_append_backward_link);
    RUN_TEST(test_append_last_entry_next_is_null);
    RUN_TEST(test_append_multiple_ids_sequential);
    RUN_TEST(test_append_preserves_full_string);
    RUN_TEST(test_append_list_length_grows);
    RUN_TEST(test_append_bidirectional_traversal_consistent);

    printf("\n-- find_in_history --\n");
    RUN_TEST(test_find_single_entry);
    RUN_TEST(test_find_last_entry);
    RUN_TEST(test_find_middle_entry);
    RUN_TEST(test_find_nonexistent_returns_null);
    RUN_TEST(test_find_returns_correct_pointer);
    RUN_TEST(test_find_returns_null_on_empty_list);

    printf("\n-- delete_from_history --\n");
    RUN_TEST(test_delete_middle_entry_not_findable);
    RUN_TEST(test_delete_middle_reconnects_list);
    RUN_TEST(test_delete_nonexistent_id_leaves_list_intact);
    RUN_TEST(test_delete_reduces_list_count);
    RUN_TEST(test_delete_first_entry);
    RUN_TEST(test_delete_last_entry);

    printf("\n-- free_history_entry --\n");
    RUN_TEST(test_free_null_does_not_crash);
    RUN_TEST(test_free_valid_detached_entry);

    printf("\n-- clear_shell_history --\n");
    RUN_TEST(test_clear_empty_history_does_not_crash);
    RUN_TEST(test_clear_single_entry_sets_globals_null);
    RUN_TEST(test_clear_multiple_entries_sets_globals_null);
    RUN_TEST(test_clear_allows_append_after_clear);
    RUN_TEST(test_clear_idempotent_on_already_empty_list);
    RUN_TEST(test_clear_list_is_not_traversable_after_clear);
    RUN_TEST(test_clear_find_returns_null_after_clear);

    printf("\n-- print_history --\n");
    RUN_TEST(test_print_empty_history_produces_no_output);
    RUN_TEST(test_print_single_entry_contains_id);
    RUN_TEST(test_print_single_entry_contains_text);
    RUN_TEST(test_print_multiple_entries_in_order);
    RUN_TEST(test_print_id_appears_before_entry_text);
    RUN_TEST(test_print_each_entry_on_separate_line);
    RUN_TEST(test_print_all_ids_present);
    RUN_TEST(test_print_preserves_special_characters);

    printf("\n-- write_history_to_file --\n");
    RUN_TEST(test_write_creates_history_file);
    RUN_TEST(test_write_empty_history_creates_empty_file);
    RUN_TEST(test_write_single_entry_file_content);
    RUN_TEST(test_write_multiple_entries_file_content);
    RUN_TEST(test_write_each_entry_on_separate_line);
    RUN_TEST(test_write_overwrites_existing_file);
    RUN_TEST(test_write_leaves_first_entry_null);

    printf("\n-- read_history_from_file --\n");
    RUN_TEST(test_read_missing_file_returns_minus_one);
    RUN_TEST(test_read_does_not_disturb_existing_history_on_failure);
    RUN_TEST(test_read_empty_file_results_in_empty_history);
    /* Tests below crash at runtime due to create_and_append empty-list bug.
     * Placed last so all preceding test results are already visible. */
    RUN_TEST(test_read_populates_history_from_file);
    RUN_TEST(test_read_correct_entry_count);
    RUN_TEST(test_read_clears_existing_history_first);

    printf("\n=== Results: %d / %d passed", tests_passed, tests_run);
    if (tests_failed)
        printf(", %d FAILED", tests_failed);
    printf(" ===\n");

    if (_failed_count > 0) {
        printf("\nFailed tests:\n");
        for (int i = 0; i < _failed_count; i++)
            printf("  - %s\n", _failed_tests[i]);
    }

    return tests_failed > 0 ? 1 : 0;
}
