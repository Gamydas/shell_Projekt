#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "input.h"
#include "str.h"
#include "shellcmd.h"
#include "escapesequenzen.h"

int main(int argc, char *argv[])
{
    char wdir[2048];                   // string for working directory 
    char cmd[200];                     // string for user command
    char* instruc[50];                 // array of strings for seperate instructions within the command
    char buffer[50][50];              // memory to assign to instruc, might be changed for dynamic memory in the future
    

    /*printf("You have entered %d arguments:\n", argc);

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    */
    CLEAR;
    HOME;
    while(1)
    {
        // Initializing instruc 
        for (int i = 0; i < 50; i++)
        {
            instruc[i] = buffer[i];
        }

        // loading the current directory into it wdir string
        getcwd(wdir, sizeof(wdir));

        // loading the user command into cmd string    
        getInput(wdir,cmd);

        char* temp = malloc(sizeof(char) * 200);
        // due to strcomp turning cmd fully lower case a copy of cmd is needed 
        strcopy(cmd, temp);   
             
        // exits the main loop if command is given                           
        if(nstrcomp(temp,"exit") == 0){                           
            free(temp);
            return 0;
        }
        free(temp);


        // seperating the command String into the seperate instructions
        if(parseStr(cmd,instruc) == -1)
        {
            fprintf(stderr, "\nerror parsing command\n");
            continue;
        }
        
        /*  Test loop to print parsed String
        for (int i = 0; i < 50; i++)
        {
            printf("Instruct #%i = %s\n", i, buffer[i]);
        }
        */
        // checking for redirection operators
        if (redirect(instruc) == 0)
        {
            continue;
        }
        
        if ( (strcomp(instruc[0], "cd")) == 0 )
        {
            cd(instruc[1]);
        } else if ( (strcomp(instruc[0], "pwd")) == 0 )
        {
            pwd();
        } else if ( (strcomp(instruc[0], "type")) == 0 )
        {
            type (instruc[1]);
        } else  // not a buildt-in
        {
            int rc = fork();
            if (rc < 0 )                                               // incase fork fails to execute
            {
                printf("fork failed\n");
                continue;
            }
            // child
            if (rc == 0)                                               
            {
                execvp(instruc[0], instruc);
                fprintf(stderr,"%s: not a command\n",instruc[0]);      // if execvp cannot find the given command
                continue;                                               // in case exec fails
            } else // parent
            {
                wait(0);                                               // waits till child dies 
            }
        } 
        
    }
    return 0;
}