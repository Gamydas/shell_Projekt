#ifndef HISTORY_H
#define HISTORY_H
#include <stdint.h>

typedef struct shHist
{
    struct shHist *next;
    struct shHist *prev;
    char *entry;
    uint16_t entry_size;
    uint16_t entry_ID;

} shHist;

shHist *shHist_create_and_append(shHist **first_entry, shHist **last_entry, char *new_entry, uint16_t entry_size);
shHist *find_in_history(shHist **first_entry, shHist **last_entry, uint16_t entry_ID);
void shHist_delete(shHist **first_entry, shHist **last_entry, shHist **discarded);
void shHist_free(shHist *entry);
int read_history_from_file(shHist **first_entry, shHist **last_entry);
int write_history_to_file(shHist *first_entry);
void print_history(shHist *first_entry);
void clear_shell_history(shHist **first_entry, shHist **last_entry);
int shHist_modify(shHist *old_entry, char *new_entry);

#endif
