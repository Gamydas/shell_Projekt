#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "str.h"
#include "history.h"
#include "parser.h"

extern shHist *first_entry;

/// @brief kills calling process
/// @param text ignores everything typed after exit
void shell_exit(char *text)
{
    // compiler complains about unused variables
    if (text != NULL) printf("Why are you passing exit a parameter???\n");
    write_history_to_file();
    clear_shell_history();
}
/// @brief function to change the current working directory
/// @param dir directory to be changed to
void cd(char *dir)
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
void pwd(char *flags)
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
void echo(char *text)
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
void type(char *text)
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

/// @brief builtin to display the command history and manipulate it with flags
///        currently implemented flags: -c  -  clears the entire history file
///                                     -w  -  writes the entire shell history to the history file
/// @param text
void history(char *text)
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
