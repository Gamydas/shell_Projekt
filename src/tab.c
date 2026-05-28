#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include "tab.h"
#include "shell.h"
#include "str.h"

/// @brief initializes a tabComp structure
/// @param tab
/// @return returns 0 on success, -1 on failure
int initialize_tab_struct(tabComp* tab)
{
    tab->matchcount = 0;
    tab->tabs = 0;
    /* this is the starting capacity for the matches array 
       this will be doubled every time it reaches the maximum */
    tab->capac = 5;
    tab->matches = malloc(tab->capac * sizeof(char*));
    if (tab->matches == NULL)
    {
        perror("malloc");
        return -1;
    }
    return 0;
}

/// @brief cleans up a tab struct and resets all its values
/// @param tab 
void cleanup_tab_struct(tabComp* tab)
{
    if (tab->matches != NULL)
    {
        // checks if there are matches to be freed
        for (int i = 0; i < tab->matchcount; i++)
        {
            free(tab->matches[i]);
        }

        free(tab->matches);
        tab->matches = NULL;
    }

    tab->tabs = 0;
    tab->capac = 5;
    tab->matchcount = 0;
}

/// @brief this function iterates over every builtin and looks for matches for the given token,
///        these matches get saved in tab->matches and the matchcount gets incremented.
///        Should the token be empty every builtin is counted as a match
/// @param tab
/// @param token
/// @param builtins array of builtins for shell
/// @param binamt amount of builtins shell currently has
/// @return 0 if everythin was succesfull, -1 if error occured
int complete_builtins(tabComp* tab, char* token, Builtin *table)
{
    // iterates over builtins array and saves matches to tab->matches
    while(table->name != NULL)  
    {
        if (str_comp(token, table->name) != -1)
        {
            int temp = str_len(table->name);
            // checks if capacity needs to be increased
            if (tab->matchcount == tab->capac - 1)  // -1 because of sentinel slot
            {
                int cntrl = increase_capacity(&tab->matches, &tab->capac, tab->capac); // doubles capacity
                if (cntrl < 0)
                {
                    return -1;
                }
            }
            // allocates space for the found match including appended space and null terminator
            tab->matches[tab->matchcount] = malloc(temp + 2);
            if (tab->matches[tab->matchcount] == NULL)
            {
                perror("malloc");
                return -1;
            }
            str_copy(table->name, tab->matches[tab->matchcount]);
            tab->matches[tab->matchcount][temp] = 32;
            tab->matches[tab->matchcount][temp + 1] = '\0';
            tab->matchcount++;
        }
        table++;
    }
    return 0;
    //handle_tab_cases(tab, token);
}


/// @brief this function iterates over all paths given by $PATH to look a matching executable
///        which will then be written into tab->matches and matchcount will be increased.
///        An empty token means every executable is a match
/// @param tab
/// @param token
/// @return 0 if everythin was succesfull, -1 if error occured
int complete_executables(tabComp* tab, char* token)
{
    int pathlen = str_len(getenv("PATH"));
    char* allpaths = malloc(pathlen + 1);  // allocates memory required (+1 for 0-byte)
    if (allpaths == NULL)
    {
        perror("malloc");
        return -1;
    }
    str_copy(getenv("PATH"), allpaths);  // makes a safely editable copy of $PATH
    char temp[2048];                    // temp string to load singular paths per iteration
    initialize_string(temp, 0, sizeof(temp));
    int count = 0;
    for (int i = 0; i < pathlen; i++)
    {
        // $PATH seperates pathes with : so if this is found a full path has been written into temp
        if (allpaths[i] == ':')
        {
            temp[count] = '\0';
            count = 0;
            int cntrl = complete_arguments(tab, token, temp, 1);
            if (cntrl < 0)
            {
                free(allpaths);  // avoids memory leak
                return -1;
            }
            initialize_string(temp, 0, sizeof(temp));  // resets temp string
        }
        else  // adds current character to the path string
        {
            temp[count] = allpaths[i];
            count++;
        }
    }
    temp[count] = '\0';  // final nullterminator
    int cntrl = complete_arguments(tab, token, temp, 1); 
    free(allpaths);
    if (cntrl < 0)
    {
        return -1;
    }
    return 0;
}

/// @brief this function opens up a directory(opendir) to iterate over via readdir()
///        and simultaniously runs a string by string comparison on the returned directories.
///        Every match gets stored in tabs->matches, directories get a '/' appended, every other
///        file gets a space appended
/// @param tab
/// @param token
/// @param path
/// @param isExec parameter to check if this function was called by complete_executables, needs to be given 1 if so
/// @return 0 if everythin was succesfull, -1 if error occured
int complete_arguments(tabComp* tab, char* token, char* path, int isExec)
{
    DIR* current = opendir(path);
    // not a directory, silent return is intended
    if (current == NULL)
    {
        return -1;
    }
    struct dirent* compare = NULL;

    int* i = &tab->matchcount;
    while ((compare = readdir(current)) != NULL)
    {
        // if completeExec is the caller, checks if the found file is executable
        if(isExec == 1)
        {
            if (compare->d_type == DT_DIR) continue; // avoids directories counting as a match
            char fullpath[PATH_MAX];
            snprintf(fullpath, PATH_MAX, "%s/%s", path, compare->d_name);
            if (access(fullpath, X_OK) < 0)
            {
                continue;
            }
        }
        if (str_comp(token, compare->d_name) != -1)
        {
            // checks if maximum capacity has been reached and doubles it if so
            if (*i == tab->capac - 1)
            {
                int cntrl = increase_capacity(&tab->matches, &tab->capac, tab->capac); // doubles capacity
                if (cntrl < 0)
                {
                    return -1;
                }
            }
            // + 2 for appended / or space and \0
            int temp = str_len(compare->d_name);
            tab->matches[*i] = malloc(temp + 2);
            if (tab->matches[*i] == NULL)
            {
                perror("malloc");
                return -1;
            }
            str_copy(compare->d_name, tab->matches[*i]);

            // appends slash to folders/directories
            if (compare->d_type == DT_DIR)
            {
                tab->matches[*i][temp] = '/';
                tab->matches[*i][temp + 1] = '\0';
            }
            else  // appends a space to all other files
            {
                tab->matches[*i][temp] = 32;
                tab->matches[*i][temp + 1] = '\0';
            }
            (*i)++;
        }
    } /* iterates over every file in the current directory
         looks for partial or complete matches, which are then
         put into the matches array, then increments matchcount */

    //handle_tab_cases(tab, token);

    closedir(current);
    return 0;
}

/// @brief this function, when called, checks the tab->tabs counter, and handles the
///        the appropriate cases
/// @param tab
void check_tab_amount(tabComp* tab)
{
    // there is only one or no match, so ring bell and do nothing
    if (tab->matchcount < 2)
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
    }
    else if (tab->tabs > 2 && tab->matchcount > 1)
    {
        // goes to next line
        printf("\r\n");
        fflush(stdout);
        // prints out all matches, the formating on this is still off, will be fixed later
        for (int i = 0; i < tab->matchcount; i++)
        {
            printf("%-15s", tab->matches[i]);
            if (i > 0 && i % 6 == 0)
            {
                printf("\r\n");
                fflush(stdout);
            }
        }

        printf("\r\n");  // new line so prompt doesnt write over the printf
        fflush(stdout);
        return;
    }
}

/// @brief function to handle the 3 possible cases of tab_completion
/// @param tab
/// @param token
void handle_tab_cases(tabComp* tab, char* token)
{
    // no match found
    if (tab->matchcount == 0)
    {
        printf("\a");
        fflush(stdout);
    }
    else if (tab->matchcount == 1)  // exactly one atch
    {
        str_copy(tab->matches[0], token);
        tab->tabs = 0;
    }
    else  // multiple matches
    {
        printf("\a");
        fflush(stdout);
        find_prefix(tab->matches, token, tab->matchcount);
    }
}

/// @brief this function handles filtering the to be completed token and dispatching it to
///        the appropriate case
/// @param tab
/// @param builtins array of builtins of the shell
/// @param binamt amount of shell builtins
/// @param command given command string, yet untokinzed
/// @param path 
/// @return 0 if success, -1 if an error occured
int tab_completion(tabComp* tab, Builtin *table, char* command, char* path)
{
    if (tab->tabs > 1)
    {
        check_tab_amount(tab);
        return 0;
    }

    int count = str_len(command) - 1;  // -1 to avoid overflows when used as index
    if (count < 0)
    {
        printf("\a");
        fflush(stdout);
        return 0;
    }

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
        }  // clears out every seperator at the begging of the command
        int cntrl = complete_builtins(tab, &command[count], table);
        if (cntrl < 0)
        {
            return -1;
        }
        /*
        if (tab->matchcount > 0)
        {
            handle_tab_cases(tab, &command[count]);
            return 0;
        }*/
        else
        {
            cntrl = complete_executables(tab, &command[count]);
            if (cntrl < 0)
            {
                return -1;
            }
            handle_tab_cases(tab, &command[count]);
            return 0;
        }
    }

    // entirely new token
    if (command[count] == 32)
    {
        int cntrl = complete_arguments(tab, &command[count + 1], path, 0);
        if (cntrl < 0)
        {
            return -1;
        }
    }
    else if (command[count] == '/')  // check for nested path
    {
        int temp = count;
        while (command[temp] != 32 && temp > 0)
        {
            temp--;
        }  // goes to beginning of token
        // nothing to complete was found
        if (temp == 0)
        {
            printf("\a");
            fflush(stdout);
            return 0;
        }
        // create a string to store the new pathname in
        // count is the position of the / seperator and temp now of the space infront
        char* pathname = malloc(str_len(command) - temp);
        if (pathname == NULL)
        {
            perror("malloc");
            return -1;
        }
        segment_str_copy(command, pathname, temp + 1, count);  // temp is now a space so + 1 to get first letter of token
        int cntrl = complete_arguments(tab, &command[count + 1], pathname, 0);
        free(pathname);
        if (cntrl < 0)
        {
            return -1;
        }
    }
    handle_tab_cases(tab, &command[count + 1]);
    return 0;
}