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

    // Initializing instruc
    for (int i = 0; i < 50; i++)
    {
        instruc[i] = buffer[i];
    }
    

    /*printf("You have entered %d arguments:\n", argc);

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    */
    CLEAR;
    HOME;
    while(1)
    {
        
        // loading the current directory into it wdir string
       
        getcwd(wdir, sizeof(wdir));

        // loading the user command into cmd string    
        getInput(wdir,cmd);

        // exits the main loop if command is given
        char* temp = malloc(sizeof(char) * 200);
        strcopy(cmd, temp);                                 // due to strcomp turning cmd fully lower case a copy of cmd is needed 
        if(strcomp(temp,"exit")){                           // causes cmd to be lower case even if cmd wasnt exit, needs to be fixed
            return 0;
        }
        free(temp);
        // seperating the command String into the seperate instructions
        parseStr(cmd,instruc);
        
        
        /*  Test loop to print parsed String
        for (int i = 0; i < 50; i++)
        {
            printf("Instruct #%i = %s\n", i, buffer[i]);
        }
        */
        
        if( (strcomp(instruc[0], "cd")) )
        {
            printf("chose cd cmd\n");
            cd(instruc[1]);
        } else if( (strcomp(instruc[0], "pwd")) )
        {
            printf("chose pwd cmd\n");
            pwd();
        } else  // not an implemented command
        {
            int rc = fork();
            if(rc < 0 ) // incase fork fails to execute
            {
                printf("fork failed\n");
                exit(1);
            }
            if(rc == 0)   // child
            {
                execvp(instruc[0], instruc);
            } else       // parent
            {
                wait(0); // waits till child dies 
            }


        }
        

    }
    return 0;
}