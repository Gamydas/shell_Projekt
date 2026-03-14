#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <termios.h>

#include "shell.h"
#include "input.h"
#include "str.h"
#include "builtins.h"
#include "escapesequenzen.h"
#include "parser.h"
#include "err.h"
#include "pipelining.h"

int main(int argc, char *argv[])
{
    ERR error = NO_ERROR;
    shell myShell;
    rawInput userInput;
    command instructions;         // parsed user instructions
    initShell(&myShell);
    CLEAR;
    HOME;
    while (1)
    {
        int control = initRaw(&userInput);
        if (control < 0)
        {
            error = INITIALIZATION_ERROR;
            printError(error, "main");
            continue;
        }
        control = initCMD(&instructions);
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
            freeRaw(&userInput);
            continue;
        }
        

        // seperating the command String into the seperate instructions
        control = parseInput(&instructions, userInput.cmd);
        if (control == -1)
        {
            cleanupCMD(&instructions);
            continue;
        }
        
        // checks if pipelining is called and if so handles it and then contiunues the main loop
        if(handlePipes(&instructions) == 0)
        {
            continue;
        }
        
        // exits main function
        if (strcomp(instructions.parsed[0], "exit") == 0)
        {
            cleanupCMD(&instructions);
            return 0;
        }
        if ((strcomp(instructions.parsed[0], "cd")) == 0)
        {
            cd(instructions.parsed[1]);
        }
        else if ((strcomp(instructions.parsed[0], "pwd")) == 0)
        {
            pwd();
        }
        else if ((strcomp(instructions.parsed[0], "type")) == 0)
        {
            type(instructions.parsed[1]);
        }
        else // not a buildt-in
        {
            int rc = fork();
            if (rc < 0) // incase fork fails to execute
            {
                perror("fork");
                cleanupCMD(&instructions);
                continue;
            }
            // child
            if (rc == 0)
            {
                // if a redirection was called this dup2s alls nessecary file descriptors and closes them
                for (int i = 0; i < instructions.rdrctns; i++)
                {
                    if (instructions.redir[i].direction == REDIR_OUT_TRUNC || instructions.redir[i].direction == REDIR_OUT_APPEND)
                    {
                        dup2(instructions.redir[i].stream, STDOUT_FILENO);
                    } else if (instructions.redir[i].direction == REDIR_ERR_TRUNC || instructions.redir[i].direction == REDIR_ERR_APPEND)
                    {
                        dup2(instructions.redir[i].stream, STDERR_FILENO);
                    } else if (instructions.redir[i].direction == REDIR_IN)
                    {
                        dup2(instructions.redir[i].stream, STDIN_FILENO);
                    }
                    close(instructions.redir[i].stream);
                }
                
                execvp(instructions.parsed[0], instructions.parsed);
                fprintf(stderr, "%s: not a command\n", instructions.parsed[0]); // if execvp cannot find the given command
                cleanupCMD(&instructions);
                exit(1);                                                    // in case exec fails
            }
            else // parent
            {
                wait(0); // waits till child dies
            }
        }
        cleanupCMD(&instructions);
    }
}