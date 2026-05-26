#include <stdio.h>
#include <stdlib.h>
#include "str.h"



/// @brief function copies contents of origin into dest
/// @param origin original message that is to be copied
/// @param dest destination of the copy command
void strcopy(char *origin, char *dest)
{
    while (*origin)
    {
        *dest = *origin;
        origin++;
        dest++;
    }
    *dest = '\0';
}

/// @brief function copies a segment of origin into dest
/// @param origin original message that is to be copied
/// @param dest destination of the copy command
/// @return returns 0 if successful and -1 if there was a problem i.e. end was greater than the actual remaining string size
int strcopySeg(char *origin, char *dest, int start, int end)
{
    // checking if an interval is valid or not
    if (start > end)
    {
        *dest = '\0';
        return -1;
    }

    while (*(origin + start) && start <= end) // runs to the end of either the string or interval
    {
        *dest = *(origin + start);
        start++;
        dest++;
    }

    if (start - 1 != end) // -1 to consider the difference caused by the last loop iteration
    {
        *dest = '\0';
        return -1;
    }

    *dest = '\0';
    return 0;
}

/// @brief converts the given Text into lowercase letters
/// @param Text
void tlc(char *Text)
{
    int i = 0;
    while (*(Text + i))
    {
        // checks if current letter is an upper case letter
        if (*(Text + i) >= 'A' && *(Text + i) <= 'Z')
        {
            *(Text + i) += 32;
        }
        i++;
    }
}

/// @brief case-sensitvely compares 2 strings
/// @param first
/// @param second
/// @return returns 0 if both strings are identical, -1 otherwise and 1 if identical to the end of the first one i.e it was shorter but identical to the end
int strcomp(char *first, char *second)
{
    int i = 0;
    while (*(first + i) && *(second + i))
    {
        if (*(first + i) != *(second + i)) // checks if first and second are identical char by char
        {
            return -1;
        }
        i++;
    }

    if (*(first + i) != *(second + i)) // checks if one of the strings was shorter and thus not identical
    {
        if(strLen(second) < strLen(first))
        {
            return -1;
        }
        return 1;
    }
    return 0;
}


/// @brief function finds the greates shared prefix in a char**, i.e a string array
///        and writes it into prefix
/// @param arr char**
/// @param prefix 
/// @param size first dimension of array
void findPrefix(char** arr, char* prefix, int size)
{
    int found = 0;                                
    for (int i = 1; i < size; i++)
    {

        for (int j = 0; j < strLen(arr[0]); j++)    
        {
            if (arr[0][j] == arr[i][j] && j == found)
            {
                prefix[j] = arr[0][j];
                found++;
            } else if(arr[0][j] != arr[i][j])
            {
                break;
            }
        }
    }
    // appends 0 to the end of found prefix, if found 0 this also just inserts a 0
    prefix[found] = '\0';
}

/// @brief counts every character in a string
/// @param text 
/// @return returns sum of characters
int strLen(char* text)
{
    int i = 0;
    while(*(text + i))
    {
        i++;
    }
    return i;
}

/// @brief deletes a char in a string and connects the two ends together
/// @param text 
/// @param idx index at which character is to be deleted
void delInStr(char* text, int idx)
{
    
    while (*(text + idx + 1))
    {
        *(text + idx) = *(text + idx + 1);
        idx++;
    }
    *(text + idx) = '\0';
    
}

/// @brief inserts a character into a string at a given index
/// @param text  
/// @param c character to be inserted
/// @param idx index at which the character is to be inserted
/// @param size length of text
void insertInStr(char* text, char c, int idx, int size)
{
    // checking if idx is within bounds of the string size(-1 to make sure character fits the string)
        if(idx >= size|| idx <0) 
    {
        fprintf(stderr, "index out of bounds\n");
        return;
    }

    int i = idx;
    // first go to end of string
    while(*(text + i))
    {
        i++;
    } 

    *(text + i + 1) = '\0'; // places nullterminator at the end of the newly sized string

    // makes a "hole" in the string and cuts of the last character if nessecary
    while(i != idx)
    {
        *(text + i) = *(text + i - 1); 
        i--;
    }
    // inserts character at the desired position
    *(text + idx) = c;

}

/// @brief function to initialize every position in a str, inspired by memset
/// @param text 
/// @param c constant to fill the string with
/// @param size size of string
void initStr(char* text, int c, int size)
{
    for (int i = 0; i < size; i++)
    {
        text[i] = c;
    }
    
}

/// @brief increases the capacity of char** i.e a sting array and reallocs new memory for that char**
/// @param array
/// @param capac current capacity that is to be increased
/// @param amt   amt that is to be added to capac
/// @return returns 0 on success and -1 on failure
int increaseCapac(char*** array, int* capac, int amt)
{
    char** temp = realloc(*array, (*capac + amt) * sizeof(char*));
    if (temp == NULL)
    {
        perror("realloc");
        return -1;
    }
    *array = temp;
    *capac += amt;  // adds amt to capac on success
    return 0;
}

/// @brief this function allocates memory the size of strlen(*origin + 1) for dest and copys origin into dest
/// @param origin 
/// @param dest 
/// @param orig_size length of origin WITHOUT Nullterm
/// @return return 0 if success, -1 if failue
int allocStrCopy(char* origin, char** dest, int orig_size)
{
    *dest =  malloc(orig_size + 1);  // +1 for \0
    if (*dest == NULL)
    {
        perror("malloc");
        return -1;
    }
    strcopy(origin, *dest);
    return 0;
}


/// @brief checks if laster character of given a given string
///        with a size of atleast 1 or more equals the given
///        character and removes it if so. Does nothing if char
///        is not the last character of string or if len < 1
/// @param string string that the char is to be removed from
/// @param c      char that is to be removed
/// @param string_size    length of string (without nullterm)
void cutFromEnd(char *string, char c, int string_size)
{
    // to avoid over/underflows
    if (string_size < 1) return;

    if (*(string + string_size - 1) == c)
    {
        *(string + string_size - 1) = '\0';
    }
}