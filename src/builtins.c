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
void shell_exit(char **text, shell *sh)
{
    // compiler complains about unused variables
    if (text != NULL) printf("Why are you passing exit a parameter???\n");
    /*
    write_history_to_file();
    clear_shell_history();
    */
}
/// @brief function to change the current working directory
/// @param dir directory to be changed to
void cd(char **dir, shell *sh)
{
    if (dir == NULL)
    {
        dir = getenv("HOME");
        if (dir == NULL)
        {
            fprintf(stderr, "failed to fetch path to home directory\n");

            return;
        }
    }
    if (str_comp(dir, "~") == 0)
    {
        dir = getenv("HOME");
        if (dir == NULL)
        {
            fprintf(stderr, "failed to fetch path to home directory\n");

            return;
        }
    }
    // if chdir failed i.e dir doesnt exist
    if (chdir(dir) == -1)
    {
        fprintf(stderr, "Directory does not exit.\n");
    }
}

/// @brief prints current working directory
void pwd(char **flags, shell *sh)
{
    if (flags != NULL)
    {
        fprintf(stderr, "unknown flag:%s \n", flags);
        return;
    }
    char temp[2048];
    printf("%s\n", getcwd(temp, 2048));
}

/// @brief prints out the given string to the terminal
/// @param text
void echo(char **text, shell *sh)
{
    if (text != NULL)
    {
        printf("%s\n", text);
    }
    else
    {
        fprintf(stderr, "nothing to echo\n");
    }
}

/// @brief tells user what type a command is
/// @param text
void type(char **text, shell *sh)
{
    if (text != NULL)
    {
        if (str_comp(text, "cd") == 0 || str_comp(text, "pwd") == 0 || str_comp(text, "echo") == 0 || str_comp(text, "type") == 0)
        {
            printf("%s is a shell buildtin\n", text);
        }
        else
        {
            fprintf(stderr, "%s: is not a buildtin\n", text);  // a check for UNIX-Tools like ls & co. will be added later
        }
    }
    else
    {
        fprintf(stderr, " : not found\n");
    }
}

/*
/// @brief builtin to display the command history and manipulate it with flags
///        currently implemented flags: -c  -  clears the entire history file
///                                     -w  -  writes the entire shell history to the history file
/// @param text
void history(char **text, shell* sh)
{
    if (text == NULL)
    {
        print_history();
    } else
    {
        // clear flag
        if (str_comp(text, "-c") == 0)
        {
            clear_shell_history();
        }
        else if (str_comp(text, "-w") == 0)
        {
            write_history_to_file();
        }
    }
}
*/






/// @brief incredibly simple hashing, adding integervalue of all (unsigned) characters in a string and using the modulo operation on that value
/// @param string string of which the total sum is to be calculated
/// @param modulo_param the number which is used to modulo the sum of characters
int hashkey_calculate(char* string, int modulo_param)
{
    int result = 0;
    while(*string)
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
    while (*table->name != NULL)
    {
        int index = 
        map[hashkey_calculate(table->name, map_size)].builtin = *table->bin;
        table++;
    }
}


/// @brief fetches a value, in this case a BinFn function pointer and returns it
/// @param map pointer to an array of type bin_Hashmap
/// @param map_size size of the map array
/// @param function name of the function that is to be looked up
/// @return pointer to function if name is valid, NULL if it is not
BinFn hashmap_poll(bin_Hashmap *map, int map_size, char* function)
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