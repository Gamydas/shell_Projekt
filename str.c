#include <stdio.h>
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

/// @brief case-insensitvely compares 2 strings
/// @param first
/// @param second
/// @return returns 0 if both strings are identical, -1 otherwise
int nstrcomp(char *first, char *second)
{
    int i = 0;
    char temp[strLen(first)];
    char temp2[strLen(second)];
    strcopy(first, temp);
    strcopy(second, temp2);
    tlc(temp);  // converts first to lowercase
    tlc(temp2); // converts second to lowercase
    while (*(temp + i) && *(temp2 + i))
    {
        if (*(temp + i) != *(temp2 + i)) // checks if first and second are identical char by char
        {
            return -1;
        }
        i++;
    }

    if (*(temp + i) != *(temp2 + i)) // checks if one of the strings was shorter and thus not identical
    {
        return -1;
    }
    return 0;
}

/// @brief case-sensitvely compares 2 strings
/// @param first
/// @param second
/// @return returns 0 if both strings are identical, -1 otherwise and 1 if identical to the end of one shorter string
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
        return 1;
    }
    return 0;
}
/* function has been retired(I found a way simpler solution)
/// @brief specific niche function to make my life easier redirecting filestreams
/// @param first
/// @param second
/// @param flag flag to be changed
/// @param number number that will be assigned to flag in case comparison is succesful
/// @return returns 0 if both strings are identical, -1 otherwise
int flagstrcomp(char* first, char* second, int* flag, int number)
{
    int i = 0;
    while(*(first + i) && *(second + i))
    {
        if(*(first + i) != *(second + i))                  // checks if first and second are identical char by char
        {
            return -1;
        }
        i++;
    }

    if(*(first + i) != *(second + i))                      // checks if one of the strings was shorter and thus not identical
    {
        return -1;
    }
    //checking which flag is eligable
        *flag = number;
    return 0;
}
*/
/// @brief function seperates a string into segments which are indicated by spaces
/// @param Text string to be seperated into segments
/// @param arr  array in which the segments are to be stored, first dimension for indices second for the parts of the string
/// @return returns the amount of seperations if parsing was successful, -1 otherwise
int parseStr(char *Text, char **arr)
{
    int i = 0;    // index variable
    int arri = 0; // array index variable
    int temp = 0; // helper variable for copystrSeg
    while (*(Text + i))
    {

        if (*(Text + i) == 32)
        {
            if (i == 0) // clears out spaces at the beginning of the STR
            {
                while (*(Text + i) == 32)
                {
                    i++;
                }
                temp = i; // remembers start point of the str
                continue;
            }
            if ((strcopySeg(Text, arr[arri], temp, i - 1))) // i - 1 so the space does not get copied aswell
            {
                return -1; // error occured while segmenting the string
            }
            arri++;
            while (*(Text + i) == 32) // clears out the remaining spaces
            {
                i++;
            }
            temp = i;
        }
        i++;
    }
    if ((strcopySeg(Text, arr[arri], temp, i - 1))) // i - 1 so the nullterminator does not get copied aswell
    {
        return -1; // error occured while segmenting the string
    }
    arri++;
    arr[arri] = NULL; // marks end of instruction array
    return arri;
}

/// @brief function finds the greates shared prefix in a 2D char arrays and writes it into prefix
/// @param arr 
/// @param prefix 
/// @param length first dimension of array
/// @param width second dimension of array
void findPrefix(char (*arr)[2048], char* prefix, int width, int length)
{
    int found = 0;                                
    int brk = 0;                                 // flag to break out of outer for loop
    for (int i = 1; i < length; i++)
    {
        if (brk)
        {
            break;
        }
        for (int j = found; j < width; j++)      // 256 comes from the calling funtion tabComplete, might make this more generally usable in the future
        {
            if (arr[0][j] == arr[i][j])
            {
                prefix[j] = arr[0][j];
                found++;
            }
            else
            {
                brk = 1;
                prefix[j] = '\0';
            }
        }
    }
    if(found == 0) // no shared prefix found
    {
        prefix[0] = '\0';
    }
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
/// @param size size of text
void insertInStr(char* text, char c, int idx, int size)
{
    // checking if idx is within bounds of the string size(-1 to make sure character fits the string)
    if(idx >= size - 1 || idx <0) 
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