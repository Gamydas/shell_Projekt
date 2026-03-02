#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "str.h"


void initList(list* list_)
{
    list_->list = malloc(2048); // creates an empty, leading item (useful for input.c)
    list_->size = 0;
}

/// @brief adds item to list
/// @param list_ 
/// @param text 
/// @return return 0 if successful and -1 if not
int addItem(list* list_, char* text)
{
    /* 
    if(!*text) // checks if given string was empty
    {
        fprintf(stderr, "addItem got handed an empty String\n");
        return -1;
    }
    */
    char (*temp)[2048] = realloc(list_->list, (list_->size + 1) * 2048);
    if(temp == NULL) // checks memory allocation error
    {
        fprintf(stderr, "reallocating memory for list failed!\n");
        return -1;
    }
    
    list_->list = temp;
    strcopy(text, list_->list[list_->size]);
    list_->size++;  // increments size after the copying due to size always being an exact representation of items in the list
    
    return 0;
}

/// @brief function to fetch a variable and if neccesary modify it
/// @param list_ 
/// @param idx 
/// @return returns the desired entry;
char* fetchItem(list* list_, int* idx)
{
    if(list_->size == 0)
    {
        return '\0';
    }
    if(*idx < 0)
    {
        *idx = 0;    // forcefully correcting index to a valid number (need this for input.c; might change later)
        return list_->list[*idx];
    } else if(*idx >= list_->size)
    {
        *idx = list_->size-1;
        return list_->list[*idx];
    }
    return list_->list[*idx];
}

void freeList(list* list_)
{
    free(list_->list);
}
