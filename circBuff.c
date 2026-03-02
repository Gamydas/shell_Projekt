#include <stdio.h>
#include "circBuff.h"
#include "str.h"

/// @brief Initializes a struct of type charCircBuff
/// @param bufr
void initializeCircBuf(charCircBuff *bufr)
{
    for (int i = 0; i < 50; i++)
    {
        bufr->buf[i][0] = '\0';
    }

    bufr->head = &bufr->buf[0];
    bufr->latest = &bufr->buf[50];
}

/// @brief adds an entry into the buffer and increments/wraps around the head pointer
/// @param bufr buffer to be written to
/// @param text string to be written
void add2Buf(charCircBuff *bufr, char *text)
{
    if (bufr->head == &bufr->buf[49]) // checks if head is at the end of the reserved buffer memory  (hardcoded size, might change this later)
    {
        strcopy(text, *bufr->head);
        bufr->head = &bufr->buf[0]; // wraps around the end
    }
    else
    {
        strcopy(text, *bufr->head);
        bufr->head++;
    }
}

/// @brief does the same thing as add2Buf but does not increment head pointer, for some niche cases to avoid need for external modification of the buffer
/// @param bufr
/// @param text
void tempadd2Buf(charCircBuff *bufr, char *text)
{
    if (bufr->head == &bufr->buf[49]) // checks if head is at the end of the reserved buffer memory  (hardcoded size, might change this later)
    {
        strcopy(text, *bufr->head);
    }
    else
    {
        strcopy(text, *bufr->head);
    }
}

/// @brief function refreshes the value of latest to position it correclty again after usage or after changes in the buffer
/// @param bufr
void resetLatest(charCircBuff *bufr)
{
    if (bufr->head == &bufr->buf[0]) // checks negative buffer overflow
    {
        bufr->latest = &bufr->buf[49]; // wraps around buffer
    }
    else
    {
        bufr->latest = bufr->head - 1;
    }
}

/// @brief fetches latest entry to buffer and decrements the pointer to point to the from this position latest entry
/// @param bufr
/// @param dest
void fetchLatest(charCircBuff *bufr, char *dest)
{
    strcopy(*bufr->latest, dest);
    if (bufr->latest == &bufr->buf[0]) // checks for a negative buffer overflow
    {
        bufr->latest = &bufr->buf[49]; // wraps around buffer
    }
    else 
    {
        bufr->latest -= 1;
    }
}

/// @brief function which goes in the opposed direction of fetch, useful if fetch is not currently 1 behind head and has not yet been reset
/// @param bufr
/// @param dest
void revFetchLatest(charCircBuff *bufr, char *dest)
{
    
        if (bufr->latest == &bufr->buf[49]) // checks for a positive buffer overflow
        {
            bufr->latest = &bufr->buf[0]; // wraps around buffer
            strcopy(*bufr->latest, dest);
        } else if(bufr->latest == bufr->head)
        {
            strcopy(*bufr->latest, dest);
        }
        else
        {
            bufr->latest++;
            strcopy(*bufr->latest, dest);
        }
    
}