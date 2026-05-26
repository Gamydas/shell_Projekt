#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "str.h"
#include "history.h"

extern shHist *first_entry;

/// @brief kills calling process
/// @param text ignores everything typed after exit
void shell_exit(char *text)
{
    clear_shell_history();
    if (text != NULL)
	    exit(0);

    exit(0);
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
    if (strcomp(dir, "~") == 0)
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
        if (strcomp(text, "cd") == 0 || strcomp(text, "pwd") == 0 || strcomp(text, "echo") == 0 || strcomp(text, "type") == 0)
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
