#include <stdio.h>
#include <stdlib.h>
#include "history.h"
#include "str.h"
#include "../lib/err.h"
// first i.e oldest
shHist *first_entry = NULL;
// last i.e newest
shHist *last_entry = NULL;

/// @brief allocates memory for a new shHist struct and appends it in the global history list.
///        caller is responsible for freeing memory
/// @param new_entry string containing the new entry to history
/// @param entry_size size of the new entry
/// @return 0 on success, -1 on error
/// @author gamydas
int create_and_append_new_hist_entry(char *new_entry, uint16_t entry_size)
{
    shHist *new = malloc(sizeof(shHist));
    if (new == NULL)
    {
        perror("malloc");
        return -1;
    }

    int cntrl = allocStrCopy(new_entry, &new->entry, entry_size);
    if (cntrl < 0)
    {
        free(new);
        return -1;
    }
    new->next = NULL;
    new->entry_size = entry_size;
    new->entry_ID = last_entry->entry_ID + 1;

    // list is empty
    if (last_entry == NULL)
    {
        new = first_entry;
        new = last_entry;
        new->prev = NULL;
        return 0;
    } 
    // list is not empty
    last_entry->next = new;
    new->prev = last_entry;
    last_entry = new;
    return 0;
}

/// @brief searches through history for the entry with the specified ID
/// @param entry_ID
/// @return pointer to struct with the entry_ID or NULL if no entry has that ID
shHist *find_in_history(uint16_t entry_ID)
{
    if(last_entry == NULL)
    {
        printError(INVALID_ARGUMENT, "history is empty");
        return NULL;
    }
    uint16_t dist_to_zero = entry_ID;
    // absolute value of entry_ID - last_entry_ID
    uint16_t dist_to_newest = (((int)entry_ID - last_entry->entry_ID) < 0) ? -(entry_ID - last_entry->entry_ID) : (entry_ID - last_entry->entry_ID);

    shHist *temp = NULL;
    // case 1: entry is closer to beginning of list
    if (dist_to_zero < dist_to_newest)
    {
        temp = first_entry;
        while (temp != NULL && temp->entry_ID != entry_ID)
        {
            temp = temp->next;
        }
    }
    else  // case 2: entry is closer to end of list
    {
        temp = last_entry;
        while (temp != NULL && temp->entry_ID != entry_ID)
        {
            temp = temp->prev;
        }
    }
    return temp;
}

/// @brief removes a shHist struct from the
/// @param entry_ID
void delete_from_history(uint16_t entry_ID)
{
    ERR error = NO_ERROR;
    shHist *temp = find_in_history(entry_ID);
    if (temp == NULL)
    {
        error = INVALID_ARGUMENT;
        printError(error, "no history entry with that ID");
        // not a critical error so no return value to indicate it needed
        return;
    }
    if (temp == first_entry)
    {
        temp->next->prev = NULL;
        first_entry = temp->next;
        free_history_entry(temp);
    }
    else if (temp == last_entry)
    {
        temp->prev->next = NULL;
        last_entry = temp->prev;
        free_history_entry(temp);
    }
    else
    {
        temp->prev->next = temp->next;
        temp->next->prev = temp->prev;
        free_history_entry(temp);
    }
}

void free_history_entry(shHist *entry)
{
    if (entry != NULL) free(entry->entry);
    free(entry);
}