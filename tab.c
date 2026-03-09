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

/// @brief checks what purpose(bin complete, exec complete, argument complete) the tab press serves
/// @param sh
/// @return returns 0 if a bin is to be completed, 1 if its an exec and 2 if its arguments, i.e files or flags(later)
int checkPurpose(shell *sh)
{
    if (sh->binflag == -1)
    {
        return 0;
    }
}

/// @brief function to handle the 3 possible cases of tabComplete
/// @param tab 
/// @param token 
void handleCases(tabComp *tab, char* token)
{
    // no match found
    if(tab->matchcount == 0)
    {
        printf("\a");
        fflush(stdout);
    } else if(tab->matchcount == 1) // exactly one atch
    {
        strcopy(tab->matches[0], token);
    } else // multiple matches
    {
        printf("\a");
        fflush(stdout);
        findPrefix(tab->matches, token, 2048, 50);
    }
}
void completeArgs(tabComp *tab, char *token, char *path)
{
    DIR *current = opendir(path);
    struct dirent *compare = NULL;

    int i = 0;
    while ((compare = readdir(current)) != NULL)
    {
        if (strcomp(token, compare->d_name) != -1)
        {
            strcopy(compare->d_name, tab->matches[i]);
            tab->matchcount++;

            int temp = strLen(compare->d_name);

            // appends slash to folders/directories
            if(compare->d_type == DT_DIR)
            {
                tab->matches[i][temp] = '/';
                tab->matches[i][temp + 1] = '\0';
            } else // appends a space to all other files
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

void tabComplete(tabComp *tab, char *command, char *path)
{
    DIR *current = opendir(path);
    struct dirent *compare = NULL;
    int count = strLen(command);

    while ((command[count] != 32  && command[count] != '/') && count > 0)
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
        char *pathname = malloc(strLen(command) - temp);
        if (pathname == NULL)
        {
            fprintf(stderr, "Error occured while mallocing\n");
        }
        strcopySeg(command, pathname, temp + 1, count); // temp is now a space so + 1 to get first letter of token
        completeArgs(tab, &command[count + 1], pathname);
        free(pathname);
    }
    closedir(current);

}