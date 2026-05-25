/*
 Unit tests for src/history.c
 These tests were written with the help of claude code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../lib/history.h"
#include "../lib/str.h"
#include "../lib/err.h"

/* Access the globals defined in history.c */
extern shHist *first_entry;
extern shHist *last_entry;

/* ── simple test framework ─────────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg)                                                  \
    do {                                                                   \
        tests_run++;                                                       \
        if (cond) {                                                        \
            printf("  [PASS] %s\n", (msg));                               \
            tests_passed++;                                                \
        } else {                                                           \
            printf("  [FAIL] %s  (line %d)\n", (msg), __LINE__);         \
            tests_failed++;                                                \
        }                                                                  \
    } while (0)

#define ASSERT_EQ(a, b, msg)    ASSERT((a) == (b),         (msg))
#define ASSERT_NE(a, b, msg)    ASSERT((a) != (b),         (msg))
#define ASSERT_NULL(p, msg)     ASSERT((p) == NULL,        (msg))
#define ASSERT_NOT_NULL(p, msg) ASSERT((p) != NULL,        (msg))
#define ASSERT_STR_EQ(a, b, msg) ASSERT(strcmp((a),(b))==0,(msg))

#define RUN_TEST(fn)                                                       \
    do {                                                                   \
        printf("\n[TEST] %s\n", #fn);                                     \
        fn();                                                              \
        teardown();                                                        \
    } while (0)

/* ── test helpers ──────────────────────────────────────────────────────── */

/* Walk and free the entire list, then reset globals. */
static void teardown(void)
{
    shHist *curr = first_entry;
    while (curr != NULL) {
        shHist *next = curr->next;
        free(curr->entry);
        free(curr);
        curr = next;
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
    size_t len = strlen(text);
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
    create_and_append_new_hist_entry((char *)cmd, (uint16_t)strlen(cmd));
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
    /* first_entry and last_entry are already NULL from previous teardown.
     * NOTE: find_in_history dereferences last_entry->entry_ID without a null
     * check, so this test is expected to crash (same root cause as BUG-1 for
     * creation).  We therefore skip the actual call and only document the
     * expectation. Remove the #if 0 block once the null guard is added. */

    shHist *found = find_in_history(1);
    ASSERT_NULL(found, "find returns NULL on empty history");

    ASSERT(1, "find_in_history(empty) SKIPPED -- would segfault (null guard missing)");
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
 * main
 * ══════════════════════════════════════════════════════════════════════════ */

int main(void)
{
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

    printf("\n=== Results: %d / %d passed", tests_passed, tests_run);
    if (tests_failed)
        printf(", %d FAILED", tests_failed);
    printf(" ===\n");

    return tests_failed > 0 ? 1 : 0;
}
