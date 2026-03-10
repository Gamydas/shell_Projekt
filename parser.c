#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

/// @brief this functions purpose is to parse a given string
///        according to a very specific set of rules
/// @param text
void parseInput(char *text, char **parsed)
{
    MODUS mode = NORMAL;
    int parseamt = 0; // amount of parsed tokens; the first dimension of a char[][]
    int idx = 0;      // length of currently parsed token; second dimension of char[][]
    while (*text)
    {
        switch (*text)
        {
        case '\t':
            // filters tabs at beginning of command
            if(parseamt == 0 && idx == 0 && mode == NORMAL)
            {
                while(*text == '\t') text++; 
                break;
            }

            // new token
            if (mode == NORMAL)
            {
                if(*(text-1) == '\t')
                {
                    text++;
                    break;
                }
                parsed[parseamt][idx] = '\0'; // terminates token 
                parseamt++;
                text++;
                idx = 0;
                break;
            }
            // space gets read as a normal sign, so it goes into the default case
            if (mode == SINGLE_QUOTES || mode == DOUBLE_QUOTES)
            {
                goto append;
            }
            break;
        case 32: // space
            // filters spaces at beginning of command
            if(parseamt == 0 && idx == 0 && mode == NORMAL)
            {
                while(*text == 32) text++; 
                break;
            }
            // new token
            if (mode == NORMAL)
            {
                if(*(text-1) == 32)
                {
                    text++;
                    break;
                }
                parsed[parseamt][idx] = '\0'; // terminates token 
                parseamt++;
                text++;
                idx = 0;
                break;
            }
            // space gets read as a normal sign, so it goes into the default case
            if (mode == SINGLE_QUOTES || mode == DOUBLE_QUOTES)
            {
                goto append;
            }
            break;
        case '"':
            switchModes(&mode,*text);
            text++;
            break;
        case 39:
            switchModes(&mode,*text);
            text++;
            break;
        /*
        case '$':
        
        

        case  39: // single quote

        case '*':

        case '?':

        case '~':
        */
        
        default:
        append: // C doesnt allow goto default; so this is the fallthrough point for most functions
            parsed[parseamt][idx] = *text;
            idx++;
            text++;
            if(!*text) parsed[parseamt][idx] = '\0'; // terminates final token
            break;
        }
    }
    parsed[parseamt + 1] = NULL; // array termination
}

/// @brief This function sets the current working mode of the parser.
///        Single and Double respond to the respective sign, every
///        other read sign will not affect the modes.
///        In Normal mode every sign is taken processed their intended
///        purpose. More documentation on special signs follows 
/// @param mode 
/// @param c 
void switchModes(MODUS *mode, char c)
{
    switch (c)
    {
    case '"':
        // switches to D_Q if current mode is normal
        if(*mode == NORMAL)
        {
            *mode = DOUBLE_QUOTES;
            
            break;
        } 
        // if in D_Q switches back to normal (might need to look this over later)
        if(*mode == DOUBLE_QUOTES)
        {
            *mode = NORMAL;
            
            break;
        }
        break;
    
    case 39: // single quote '
        // switches to S_Q if current mode is normal
        if(*mode == NORMAL)
        {
            *mode = SINGLE_QUOTES;
            
            break;
        } 
        // if in S_Q switches back to normal (might need to look this over later)
        if(*mode == SINGLE_QUOTES)
        {
            *mode = NORMAL;
            
            break;
        }
        break;
    default:
        break;
    }
}