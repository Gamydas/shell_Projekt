#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "history.h"
#include "str.h"
#include "../lib/err.h"

/// @brief allocates memory for a new shHist struct and appends it in the global history list.
///        caller is responsible for freeing memory
/// @param first_entry pointer to first item in linked list
/// @param last_entry pointer to last item in linked list
/// @param new_entry string containing the new entry to history
/// @param entry_size size of the new entry
/// @return pointer to new struct, -1 on error
/// @author gamydas
shHist *shHist_create_and_append(shHist **first_entry, shHist **last_entry, char *new_entry, uint16_t entry_size)
{
    shHist *new = malloc(sizeof(shHist));
    if (new == NULL)
    {
        perror("malloc");
        return NULL;
    }

    int cntrl = alloc_str_copy(new_entry, &new->entry, entry_size);
    if (cntrl < 0)
    {
        free(new);
        return NULL;
    }
    new->next = NULL;
    new->entry_size = entry_size;

    // list is empty
    if (*last_entry == NULL)
    {
        *first_entry = new;
        *last_entry = new;
        new->entry_ID = 1;
        new->prev = NULL;
        return new;
    }
    // list is not empty
    new->entry_ID = (*last_entry)->entry_ID + 1;
    (*last_entry)->next = new;
    new->prev = *last_entry;
    *last_entry = new;
    return new;
}

/// @brief searches through history for the entry with the specified ID
/// @param first_entry pointer to first item in linked list
/// @param last_entry pointer to last item in linked list
/// @param entry_ID
/// @return pointer to struct with the entry_ID or NULL if no entry has that ID
shHist *find_in_history(shHist **first_entry, shHist **last_entry, uint16_t entry_ID)
{
    if (last_entry == NULL)
    {
        print_error(INVALID_ARGUMENT, "history is empty");
        return NULL;
    }
    uint16_t dist_to_zero = entry_ID;
    // absolute value of entry_ID - last_entry_ID
    uint16_t dist_to_newest = (((int)entry_ID - (*last_entry)->entry_ID) < 0) ? -(entry_ID - (*last_entry)->entry_ID) : (entry_ID - (*last_entry)->entry_ID);

    shHist *temp = NULL;
    // case 1: entry is closer to beginning of list
    if (dist_to_zero < dist_to_newest)
    {
        temp = *first_entry;
        while (temp != NULL && temp->entry_ID != entry_ID)
        {
            temp = temp->next;
        }
    }
    else  // case 2: entry is closer to end of list
    {
        temp = *last_entry;
        while (temp != NULL && temp->entry_ID != entry_ID)
        {
            temp = temp->prev;
        }
    }
    return temp;
}

/// @brief removes a shHist struct from the global linked list
/// @param first_entry pointer to adress of first item in linked list
/// @param last_entry pointer to adress of last item in linked list
/// @param discarded pointer to item that is to be deleted
void shHist_delete(shHist **first_entry, shHist **last_entry, shHist **discarded)
{
    ERR error = NO_ERROR;
    if (*discarded == NULL)
    {
        error = INVALID_ARGUMENT;
        print_error(error, "no history entry with that ID");
        // not a critical error so no return value to indicate it needed
        return;
    }
    // discarded is the only history item
    if ((*discarded)->prev == NULL && (*discarded)->next == NULL)
    {
        shHist_free(*discarded);
        *discarded = NULL;
        *first_entry = NULL;
        *last_entry = NULL;
        return;
    }
    // discarded item is first entry in list
    if ((*discarded)->prev == NULL)
    {
        *first_entry = (*discarded)->next;
        (*first_entry)->prev = NULL;
        shHist_free(*discarded);
        *discarded = NULL;
    }  // discarded item is last_entry in list
    else if ((*discarded)->next == NULL)
    {
        *last_entry = (*discarded)->prev;
        (*last_entry)->next = NULL;
        shHist_free(*discarded);
        *discarded = NULL;
    }
    else
    {
        (*discarded)->prev->next = (*discarded)->next;
        (*discarded)->next->prev = (*discarded)->prev;
        shHist_free(*discarded);
        *discarded = NULL;
    }
}

void shHist_free(shHist *entry)
{
    if (entry != NULL) free(entry->entry);
    free(entry);
}

/// @brief reads history from the .myshell_history file in the home directory
/// @param first_entry pointer to first item in linked list
/// @param last_entry pointer to last item in linked list
/// @return 0 on success, -1 on failure
int read_history_from_file(shHist *first_entry, shHist *last_entry)
{
    // fetches path to home directory
    char *home = getenv("HOME");
    if (home == NULL)
    {
        perror("getenv HOME");
        return -1;
    }

    // creates formatstring with total path to home directorys myshell_hist file
    char path[4096];
    snprintf(path, sizeof(path), "%s/.myshell_history", home);

    FILE *stream = fopen(path, "r");
    if (stream == NULL)
    {
        perror("fopen");
        return -1;
    }

    // clears current history incase its not 0 to avoid colissions or
    // duplicate entries
    if (first_entry != NULL)
    {
        clear_shell_history(&first_entry);
    }

    // size of line has to be increased eventually, this is just for testing
    char *line = malloc(1024);
    while (fgets(line, 1024, stream) != NULL)
    {
        // removing potential white spaces from the end of the line that cause parsing errors
        cut_character_from_end(line, '\n', str_len(line));
        cut_character_from_end(line, '\r', str_len(line));
        // this might cause problems cause of 128/lengthchecks
        if (shHist_create_and_append(&first_entry, &last_entry, line, 1024) == NULL)
        {
            print_error(INITIALIZATION_ERROR, "create history entry");
            return -1;
        }
        // might need to remove \r\n
    }
    free(line);
    fclose(stream);
    return 0;
}

/// @brief writes the entire history to .myshell_history, which will
///        be located in your home directory
/// @param first_entry pointer to first item in linked list
/// @return 0 if success, -1 if failure
int write_history_to_file(shHist *first_entry)
{
    char *home = getenv("HOME");
    if (home == NULL)
    {
        perror("getenv HOME");
        return -1;
    }

    // creating aboslute path to home directory
    char path[4096];
    snprintf(path, sizeof(path), "%s/.myshell_history", home);

    FILE *stream = fopen(path, "w");
    if (stream == NULL)
    {
        perror("fopen");
        return -1;
    }

    // iterates over the entire history entry by entry
    // writes it exactly into the destination file(.myshell_history)
    shHist *temp = first_entry;
    while (temp != NULL)
    {
        fprintf(stream, "%s\n", temp->entry);
        temp = temp->next;
    }

    fclose(stream);
    return 0;
}

/// @brief prints the entire history of the shell
/// @param first_entry pointer to first item in linked list
void print_history(shHist *first_entry)
{
    if (first_entry == NULL) return;

    shHist *temp = first_entry;
    while (temp != NULL)
    {
        printf("%d  %s\n", temp->entry_ID, temp->entry);
        temp = temp->next;
    }
}

/// @brief clears the entire shells history and frees all allocated memeory
///        caller file needs to include the global first_entry variable
/// @param first_entry pointer to first item in linked list
void clear_shell_history(shHist **first_entry)
{
    shHist *temp = NULL;
    if (*first_entry != NULL) temp = (*first_entry)->next;
    while (temp != NULL)
    {
        shHist_free(*first_entry);
        *first_entry = temp;
        temp = temp->next;
    }
    shHist_free(*first_entry);
    *first_entry = NULL;
    // last_entry = NULL;
}

/// @brief modifies an item of the shHist dlinked lisit
/// @param old_entry pointer to the struct of which the content is to be changed
/// @param new_entry new entry to be written in the given entry_ID
/// @return 0 on success, -1 if error occured
int shHist_modify(shHist *old_entry, char *new_entry)
{
    free(old_entry->entry);
    int len = str_len(new_entry);
    int cntrl = alloc_str_copy(new_entry, &old_entry->entry, len);
    if (cntrl < 0) return -1;
    // resizing entry
    old_entry->entry_size = len;
    return 0;
}