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


/// @brief converts the given Text into lowercase letters
/// @param Text 
void tlc(char* Text)
{
    int i = 0;
    while(*(Text + i))
    {
        // checks if current letter is part of the upper case letters
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

