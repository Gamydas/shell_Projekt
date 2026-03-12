#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <termios.h>

#include "shell.h"
#include "input.h"
#include "str.h"
#include "buildtins.h"
#include "escapesequenzen.h"
#include "parser.h"
#include "err.h"

int main(int argc, char *argv[])
{
    ERR error = NO_ERROR;
    shell myShell;
    command userInput;
    initShell(&myShell);
    CLEAR;
    HOME;
    while (1)
    {
        int control = initCMD(&userInput);
        if (control < 0)
        {
            error = INITIALIZATION_ERROR;
            printError(error, "main");
            continue;
        }
        // loading the current directory into it wdir string
        getcwd(myShell.wdir, sizeof(myShell.wdir));

        // loading the user command into cmd string
        control = getInput(&myShell, &userInput);
        if (control == -1)
        {
            cleanupCMD(&userInput);
            continue;
        }
        

        // seperating the command String into the seperate instructions
        control = parseInput(&userInput);
        if (control == -1)
        {
            cleanupCMD(&userInput);
            continue;
        }
        /*
        // checking for redirection operators
        // checks if pipelining is called and if so handles it and then contiunues the main loop
        if(handlePipes(&myShell) == 0)
        {
            continue;
        }
        */
        // exits main function
        if (strcomp(userInput.parsed[0], "exit") == 0)
        {
            cleanupCMD(&userInput);
            return 0;
        }
        if ((strcomp(userInput.parsed[0], "cd")) == 0)
        {
            cd(userInput.parsed[1]);
        }
        else if ((strcomp(userInput.parsed[0], "pwd")) == 0)
        {
            pwd();
        }
        else if ((strcomp(userInput.parsed[0], "type")) == 0)
        {
            type(userInput.parsed[1]);
        }
        else // not a buildt-in
        {
            int rc = fork();
            if (rc < 0) // incase fork fails to execute
            {
                perror("fork");
                cleanupCMD(&userInput);
                continue;
            }
            // child
            if (rc == 0)
            {
                // if a redirection was called this dup2s alls nessecary file descriptors and closes them
                for (int i = 0; i < userInput.rdrctns; i++)
                {
                    if (userInput.redir[i].direction == REDIR_OUT_TRUNC || userInput.redir[i].direction == REDIR_OUT_APPEND)
                    {
                        dup2(userInput.redir[i].stream, STDOUT_FILENO);
                    } else if (userInput.redir[i].direction == REDIR_ERR_TRUNC || userInput.redir[i].direction == REDIR_ERR_APPEND)
                    {
                        dup2(userInput.redir[i].stream, STDERR_FILENO);
                    } else if (userInput.redir[i].direction == REDIR_IN)
                    {
                        dup2(userInput.redir[i].stream, STDIN_FILENO);
                    }
                    close(userInput.redir[i].stream);
                }
                
                execvp(userInput.parsed[0], userInput.parsed);
                fprintf(stderr, "%s: not a command\n", userInput.parsed[0]); // if execvp cannot find the given command
                cleanupCMD(&userInput);
                exit(1);                                                    // in case exec fails
            }
            else // parent
            {
                wait(0); // waits till child dies
            }
        }
        cleanupCMD(&userInput);
    }
}