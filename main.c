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
//#include "list.h"

int main(int argc, char *argv[])
{
    shell myShell;
    initShell(&myShell);
    CLEAR;
    HOME;
    while (1)
    {
        // loading the current directory into it wdir string
        getcwd(myShell.wdir, sizeof(myShell.wdir));

        // loading the user command into cmd string
        getInput(&myShell);

        // Initializing instruc
        for (int i = 0; i < 50; i++)
        {
            myShell.instruc[i] = myShell.buffer[i];
        }

        

        // seperating the command String into the seperate instructions
        parseInput(myShell.cmd, myShell.instruc);

        /*  Test loop to print parsed String
        for (int i = 0; i < 50; i++)
        {
            printf("Instruct #%i = %s\n", i, buffer[i]);
        }
        */
        // checking for redirection operators
        if (redirect(&myShell) == 0)
        {
            continue;
        }
        // checks if pipelining is called and if so handles it and then contiunues the main loop
        if(handlePipes(&myShell) == 0)
        {
            continue;
        }
        // exits main function
        if (strcomp(myShell.cmd, "exit") == 0)
        {
            return 0;
        }
        if ((strcomp(myShell.instruc[0], "cd")) == 0)
        {
            cd(myShell.instruc[1]);
        }
        else if ((strcomp(myShell.instruc[0], "pwd")) == 0)
        {
            pwd();
        }
        else if ((strcomp(myShell.instruc[0], "type")) == 0)
        {
            type(myShell.instruc[1]);
        }
        else // not a buildt-in
        {
            int rc = fork();
            if (rc < 0) // incase fork fails to execute
            {
                fprintf(stderr, "fork failed\n");
                continue;
            }
            // child
            if (rc == 0)
            {
                execvp(myShell.instruc[0], myShell.instruc);
                fprintf(stderr, "%s: not a command\n", myShell.instruc[0]); // if execvp cannot find the given command
                exit(1);                                                    // in case exec fails
            }
            else // parent
            {
                wait(0); // waits till child dies
            }
        }
    }
}