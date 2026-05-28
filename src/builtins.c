#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lib/builtins.h"
#include "str.h"
#include "history.h"
#include "parser.h"
#include "shell.h"

/// @brief kills calling process
/// @param text ignores everything typed after exit
int shell_exit(char **text, shell *sh)
{
    (void)sh;
    if (*text != NULL) printf("Why are you passing exit a parameter???\n");
    /*
    write_history_to_file();
    clear_shell_history();
    */
    return 0;
}
/// @brief function to change the current working directory
/// @param dir directory to be changed to
/// @param 0 on success, -1 on if directory doesnt exist, -2 if $HOME is not set
int cd(char **dir, shell *sh)
{
    (void)sh;
    // empty argument
    if (*dir == NULL)
    {
        *dir = getenv("HOME");
        if (*dir == NULL)
        {
            fprintf(stderr, "failed to fetch path to home directory\n");

            return -2;
        }
    }
    else if (str_comp(*dir, "~") == 0)
    {
        *dir = getenv("HOME");
        if (*dir == NULL)
        {
            fprintf(stderr, "failed to fetch path to home directory\n");

            return -2;
        }
    }
    // if chdir failed i.e dir doesnt exist
    if (chdir(*dir) == -1)
    {
        fprintf(stderr, "Directory does not exit.\n");
        return -1;
    }
    return 0;
}

/// @brief prints current working directory
/// @param 0 normally, -1 if unkown flag is passed
int pwd(char **flags, shell *sh)
{
    (void)sh;
    int flag = 0;
    if (*flags != NULL) flag = -1;
    while (*flags != NULL)
    {
        if (str_comp(*flags, "-L") == 0)
        {
            flag = 1;
        }
    }
    if (flag == -1)
    {
        fprintf(stderr, "unknown flag:%s \n", *flags);
        return -1;
    }
    char temp[2048];
    printf("%s\n", getcwd(temp, 2048));
    return 0;
}

/// @brief tells user what type a command is
/// @param text
/// @param 0 if known type, -1 if unkown
int type(char **text, shell *sh)
{
    // currently ignores possible flags
    if (*text != NULL)
    {
        if (hashmap_poll(sh->builtins, 50, *text) != NULL)
        {
            printf("%s is a shell buildtin\n", *text);
        }
        else
        {
            fprintf(stderr, "%s: is not a buildtin\n", *text);  // a check for UNIX-Tools like ls & co. will be added later
        }
    }
    else
    {
        fprintf(stderr, " : not found\n");
        return -1;
    }
    return 0;
}

/// @brief builtin to display the command history and manipulate it with flags
///        currently implemented flags: -c  -  clears the entire history file
///                                     -w  -  writes the entire shell history to the history file
/// @param text
/// @param sh  pointer to current shell struct
/// @return 0 on success, -1 if unkown flag is passed
int history(char **text, shell *sh)
{
    if (*text == NULL)
    {
        print_history(sh->first_entry);
        return 0;
    }
    else
    {
        while (*text != NULL)
        {
            // clear flag
            if (str_comp(*text, "-c") == 0)
            {
                clear_shell_history(&sh->first_entry, &sh->last_entry);
            }
            else if (str_comp(*text, "-w") == 0)
            {
                write_history_to_file(sh->first_entry);
            }
            else
            {
                fprintf(stderr, "unknown flag:%s \n", *text);
                return -1;
            }
            text++;
        }
    }
    return 0;
}

/// @brief incredibly simple hashing, adding integervalue of all (unsigned) characters in a string and using the modulo operation on that value
/// @param string string of which the total sum is to be calculated
/// @param modulo_param the number which is used to modulo the sum of characters
int hashkey_calculate(char *string, int modulo_param)
{
    int result = 0;
    while (*string)
    {
        result += *string;
        string++;
    }
    return result % modulo_param;
}

/// @brief populates a hashmap with a simple hashing function
/// @param map pointer to an array of type bin_Hashmap
/// @param map_size size of the map array
/// @param table pointer to functiontable of builtins
void hashmap_populate(bin_Hashmap *map, int map_size, Builtin *table)
{
    while (table->name != NULL)
    {
        
        map[hashkey_calculate(table->name, map_size)].builtin = table->bin;
        table++;
    }
}

/// @brief fetches a value, in this case a BinFn function pointer and returns it
/// @param map pointer to an array of type bin_Hashmap
/// @param map_size size of the map array
/// @param function name of the function that is to be looked up
/// @return pointer to function if name is valid, NULL if it is not
BinFn hashmap_poll(bin_Hashmap *map, int map_size, char *function)
{
    BinFn funct = map[hashkey_calculate(function, map_size)].builtin;
    return (funct != NULL) ? funct : NULL;
}

/// @brief sets contents of all map nodes to 0
/// @param map pointer to an array of type bin_Hashmap
/// @param map_size size of map
void hashmap_initialize(bin_Hashmap *map, int map_size)
{
    for (int i = 0; i < map_size; i++)
    {
        map[i].builtin = NULL;
    }
}