#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "tab.h"
#include "str.h"
#include "shell.h"

/// @brief initializes a tabComp structure
/// @param tab
void initTab(tabComp *tab)
{
    tab->matchcount = 0;
    tab->tabs = 0;
    for (int i = 0; i < 50; i++)
    {
        initStr(tab->matches[i], 0, 2048);
    }
}

/// @brief this function iterates over every builtin and looks for matches for the given token,
///        these matches get saved in tab->matches and the matchcount gets incremented. 
///        Should the token be empty every builtin is counted as a match
/// @param tab 
/// @param token 
/// @param builtins 
void completeBuiltins(tabComp *tab, char *token, char (*builtins)[10])
{
    // iterates over builtins array and saves matches to tab->matches
    for (int i = 0; i < 4; i++) // INCREASE THIS IF U ADD MORE BUILTINS!
    {
        if (strcomp(builtins[i], token) != -1)
        {
            int temp = strLen(builtins[i]);
            strcopy(builtins[i], tab->matches[tab->matchcount]);
            tab->matches[tab->matchcount][temp] = 32;
            tab->matches[tab->matchcount][temp + 1] = '\0';
            tab->matchcount++;
        }
    }
    handleCases(tab, token);
}

/// @brief function to handle the 3 possible cases of tabComplete
/// @param tab
/// @param token
void handleCases(tabComp *tab, char *token)
{
    // no match found
    if (tab->matchcount == 0)
    {
        printf("\a");
        fflush(stdout);
    }
    else if (tab->matchcount == 1) // exactly one atch
    {
        strcopy(tab->matches[0], token);
    }
    else // multiple matches
    {
        printf("\a");
        fflush(stdout);
        findPrefix(tab->matches, token, 2048, 50);
    }
}
/// @brief this function iterates over all paths given by $PATH to look a matching executable
///        which will then be written into tab->matches and matchcount will be increased.
///        An empty token means every executable is a match
/// @param tab 
/// @param token 
void completeExecs(tabComp *tab, char *token)
{
    char* allpaths = malloc(strLen(getenv("PATH")) + 1); // allocates memory required (+1 for 0-byte)
    if(allpaths == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        return;
    }
    strcopy(getenv("PATH"), allpaths);          // makes a safely editable copy of $PATH
    char temp[2048];                            // temp string to load singular paths per iteration
    initStr(temp, 0, sizeof(temp));
    int count = 0;
    for (int i = 0; i < strLen(getenv("PATH")); i++)
    {
        // $PATH seperates pathes with : so if this is found a full path has been written into temp
        if(allpaths[i] == ':')
        {
            temp[count] = '\0';
            count = 0;
            completeArgs(tab, token, temp);
            initStr(temp, 0, sizeof(temp));
        } else // adds current character to the path string
        {
            temp[count] = allpaths[i];
            count++;
        }
        
    }
    temp[count] = '\0'; // final nullterminator
    completeArgs(tab, token, temp);
    free(allpaths);
}

void completeArgs(tabComp *tab, char *token, char *path)
{
    DIR *current = opendir(path);
    // not a directory
    if(current == NULL)
    {
        return;
    }
    struct dirent *compare = NULL;

    int i = tab->matchcount;
    while ((compare = readdir(current)) != NULL)
    {
        if (strcomp(token, compare->d_name) != -1)
        {
            strcopy(compare->d_name, tab->matches[i]);
            tab->matchcount++;

            int temp = strLen(compare->d_name);

            // appends slash to folders/directories
            if (compare->d_type == DT_DIR)
            {
                tab->matches[i][temp] = '/';
                tab->matches[i][temp + 1] = '\0';
            }
            else // appends a space to all other files
            {
                tab->matches[i][temp] = 32;
                tab->matches[i][temp + 1] = '\0';
            }
            i++;
        }
    } /* iterates over every file in the current directory
         looks for partial or complete matches, which are then
         put into the matches array, then increments matchcount */

    handleCases(tab, token);

    closedir(current);
}

void tabComplete(tabComp *tab, char (*builtins)[10], char *command, char *path)
{ 
    // there is only one match, so ring bell and do nothing
    if (tab->tabs > 1 && tab->matchcount == 1)
    {
        printf("\a");
        fflush(stdout);
        return;
    }
    // multiple matches, ring bell and wait for next input
    if (tab->tabs == 2 && tab->matchcount > 1)
    {
        printf("\a");
        fflush(stdout);
        return; 
    } else if (tab->tabs > 2 && tab->matchcount > 1)
    {
        // goes to next line
        printf("\r\n");
        fflush(stdout);
        // prints out all matches, the formating on this is still off, will be fixed later
        for (int i = 0; i < tab->matchcount; i++)
        {
            printf("%-15s", tab->matches[i]);
            if(i > 0 && i%6 == 0)
            {
                printf("\r\n");
                fflush(stdout);
            }
        }

        printf("\r\n");              // new line so prompt doesnt write over the printf
        fflush(stdout);
        return;
    }
    


    int count = strLen(command) - 1; // -1 to avoid overflows when used as index

    while ((command[count] != 32 && command[count] != '/') && count > 0)
    {
        count--;
    } /* looks for last typed seperator(space or slash) to
        the beginning of the current token */

    // no seperator typed, has to be redirect, bin or executable
    if (count == 0)
    {
        while (command[count] == 32 || command[count] == '/')
        {
            count++;
        } // clears out every seperator at the begging of the command
        completeBuiltins(tab, &command[count], builtins);
        if(tab->matchcount > 0)
        {
            return;
        } else
        {
            completeExecs(tab, &command[count]);
            return;
        } 
    }

    // entirely new token
    if (command[count] == 32)
    {
        completeArgs(tab, &command[count + 1], path);
    }
    else if (command[count] == '/')
    {
        int temp = count;
        while (command[temp] != 32 && temp > 0)
        {
            temp--;
        } // goes to beginning of token
        // nothing to complete wa found
        if (temp == 0)
        {
            printf("\a");
            fflush(stdout);
            return;
        }
        // create a string to store the new pathname in
        // count is the position of the / seperator and temp now of the space infront
        char *pathname = malloc(strLen(command) - temp);
        if (pathname == NULL)
        {
            fprintf(stderr, "Error occured while mallocing\n");
        }
        strcopySeg(command, pathname, temp + 1, count); // temp is now a space so + 1 to get first letter of token
        completeArgs(tab, &command[count + 1], pathname);
        free(pathname);
    }
}