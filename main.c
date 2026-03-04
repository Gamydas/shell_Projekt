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
#include "list.h"

int main(int argc, char *argv[])
{
    /*
    char wdir[2048];                   // string for working directory
    char cmd[200];                     // string for user command
    char* instruc[50];                 // array of strings for seperate instructions within the command
    char buffer[50][50];              // memory to assign to instruc, might be changed for dynamic memory in the future

    struct termios canon;
    struct termios raw;
    tcgetattr(0, &canon);
    tcgetattr(0, &raw);


    */
    shell myShell;
    myShell.latest = 0;
    initList(&myShell.history);
    tcgetattr(0, &myShell.canon);
    tcgetattr(0, &myShell.raw);
    myShell.raw.c_iflag &= ~(IXON);
    myShell.raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

    CLEAR;
    HOME;
    while (1)
    {
        // Initializing instruc
        for (int i = 0; i < 50; i++)
        {
            myShell.instruc[i] = myShell.buffer[i];
        }

        // loading the current directory into it wdir string
        getcwd(myShell.wdir, sizeof(myShell.wdir));

        // loading the user command into cmd string
        getInput(&myShell);

        // seperating the command String into the seperate instructions
        if (parseStr(myShell.cmd, myShell.instruc) == -1)
        {
            fprintf(stderr, "error parsing command\n");
            continue;
        }

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
            freeList(&myShell.history);
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
                freeList(&myShell.history);
                exit(1);                                                    // in case exec fails
            }
            else // parent
            {
                wait(0); // waits till child dies
            }
        }
    }
}