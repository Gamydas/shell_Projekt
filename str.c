#include <stdio.h>

/// @brief function copies contents of origin into dest
/// @param origin original message that is to be copied
/// @param dest destination of the copy command
void strcopy(char* origin, char* dest)
{
    while(*origin)
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
/// @return returns 0 if successful and 1 if there was a problem i.e. end was greater than the actual remaining string size
int strcopySeg(char* origin, char* dest, int start, int end)
{
    // checking if an interval is valid or not
    if(start > end)                                           
        {
            *dest = '\0';
            return 1;
        }

    while (*(origin + start) && start <= end)                                    // runs to the end of either the string or interval
    {
        *dest = *(origin + start);
        start++;
        dest++;
    }

    if(start - 1 != end)                                                        // -1 to consider the difference caused by the last loop iteration 
    {
        *dest = '\0';
        return 1;
    }

    *dest = '\0';
    return 0;
}

/// @brief converts the given Text into lowercase letters
/// @param Text 
void tlc(char* Text)
{
    int i = 0;
    while(*(Text + i))
    {
        // checks if current letter is an upper case letter
        if(*(Text + i) >= 'A' && *(Text + i) <= 'Z')                        
        {
            *(Text + i) += 32;
        }
        i++;
    }
}

/// @brief case-insensitvely compares 2 strings 
/// @param first 
/// @param second 
/// @return returns 1 if both strings are identical, 0 otherwise
int strcomp(char* first, char* second)
{
    int i = 0;
    tlc(first);                                            // converts first to lowercase
    tlc(second);                                           // converts second to lowercase
    while(*(first + i) && *(second + i))
    {
        if(*(first + i) != *(second + i))                  // checks if first and second are identical char by char
        {
            return 0;
        }
        i++;
    }

    if(*(first + i) != *(second + i))                      // checks if one of the strings was shorter and thus not identical 
    {
        return 0;
    }
    return 1;
}

/// @brief function seperates a string into segments which are indicated by spaces
/// @param Text string to be seperated into segments
/// @param arr  array in which the segments are to be stored, first dimension for indices second for the parts of the string
/// @return returns 1 if segmentation was unsuccessful, 0 otherwise
int sepStr(char* Text, char** arr)
{
    int i = 0;                                             // index variable
    int arri = 0;                                          // array index variable
    int temp = 0;                                          // helper variable for copystrSeg
    while(*(Text + i))
    {
        
        if(*(Text + i) == 32)
        {
            if(i == 0)                                     // clears out spaces at the beginning of the STR
            {
                while(*(Text + i) == 32)
                {
                    i++;
                }
                temp = i;                                  // remembers start point of the str
                continue;
            }

            if((strcopySeg(Text, arr[arri], temp, i - 1)))      // i - 1 so the space does not get copied aswell
            {
                return 1;                                       // error occured while segmenting the string
            }
            arri++;
            while (*(Text + i) == 32)                          // clears out the remaining spaces 
            {
                i++;
            }
            temp = i;
        }
        i++;
    }

    if((strcopySeg(Text, arr[arri], temp, i - 1)))              // i - 1 so the nullterminator does not get copied aswell
    {
        return 1;                                               // error occured while segmenting the string
    }
    return 0;
}

