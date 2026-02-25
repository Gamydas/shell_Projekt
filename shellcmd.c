#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "tools.h"
#include "str.h"

/// @brief function to change the current working directory
/// @param dir directory to be changed to
void cd(char* dir)
{
    if(strcomp(dir,"~") == 0)
    {
        dir = getenv("HOME");
        if(dir == NULL)
        {
            fprintf(stderr,"failed to fetch path to home directory\n");
            return;
        }
    }
    // if chdir failed i.e dir doesnt exist
    if(chdir(dir) == -1)
    {
        fprintf(stderr,"Directory does not exit.\n");
    }
}

/// @brief prints current working directory
void pwd()
{
    char* temp = malloc(200);
    printf("%s\n", getcwd(temp,200));
    free(temp);
}

/// @brief prints out the given string to the terminal
/// @param text 
void echo(char* text)
{
    printf("%s\n", text);
}

/// @brief tells user what type a command is
/// @param text 
void type(char* text)
{
    if(strcomp(text,"cd") == 0 || strcomp(text,"pwd") == 0 || strcomp(text,"echo") == 0 || strcomp(text,"type") == 0 )
    {
        printf("%s is a shell buildtin\n", text);
    } else
    {
        fprintf(stderr,"%s: is not a buildtin\n", text); // a check for UNIX-Tools like ls & co. will be added later
    }
}

/// @brief redirects stream to a chosen file 
/// @param text
/// @param filename
/// @return returns 0 if redirection was succesful, -1 otherwise
int redirect(char** instruc)
{
    int i = 0;
    
    // flags used to mark the position(and check its existence) of the operator in the string
    int flag  = -1;        // flag for > 
    int flag2 =  -1;       // flag for >>
    int flag3 =  -1;       // flag for 2>
    int flag4 =  -1;       // flag for 2>>
    int flag5 =  -1;       // flag for <

    // variables used to store file descriptors 
    int dataout = STDOUT_FILENO;      
    int dataerr = STDERR_FILENO;
    int datain  = STDIN_FILENO; 
    while(instruc[i] != NULL)
    {
        if(!strcomp(instruc[i],">"))
        {
            flag = i;
        }
        if(!strcomp(instruc[i],">>"))
        {
            flag2 = i;
        }
        if(!strcomp(instruc[i],"2>"))
        {
            flag3 = i;
        }
        if(!strcomp(instruc[i],"2>>"))
        {
            flag4 = i;
        }
        if(!strcomp(instruc[i],"<"))
        {
            flag5 = i;
        }
        i++;
    }
    
    // no redirection operator has been called
    if(flag == -1 && flag2 == -1 && flag3 == -1 && flag4 == -1 && flag5 == -1)
    {
        return -1; 
    }
    
    // fetching filedescriptors in appropriate modes
    if(flag != -1)
    {
        dataout = open(instruc[flag+1],O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if(flag2 != -1)
    {
        if(dataout == STDOUT_FILENO)
        {
            dataout = open(instruc[flag2+1],O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
    }
    if(flag3 != -1)
    {
        dataerr = open(instruc[flag3+1],O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if(flag4 != -1)
    {
        if(dataerr == STDERR_FILENO)
        {
            dataerr = open(instruc[flag4+1],O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
    }
    if(flag5 != -1)
    {
        datain = open(instruc[flag5+1],O_RDONLY);
    }

    int rc = fork();
    if(rc < 0)
    {
        fprintf(stderr,"fork failed\n");
        return -1;
    }

    // child
    if(rc == 0)
    {
        
        // setting up streams
        dup2(dataout, STDOUT_FILENO);
        dup2(dataerr, STDERR_FILENO);
        dup2(datain, STDIN_FILENO);
        close(dataout);
        close(dataerr);
        close(datain);

        // command is first in the string
        int arr[] = {flag, flag2, flag3, flag4, flag5};
        int min = findMinXn1(arr, 5);
        if(min != 0 )
        {
            // this block cuts the instruc array off at the point of redirection so ls does not read invalid arguments
            instruc[min] = NULL;

            execvp(instruc[0], instruc);
            fprintf(stderr,"%s: not a command\n",instruc[0]);      // if execvp cannot find the given command
            return -1;
        } 
        // redirection instructions come first
        if(min == 0)
        {
    
            int max = findMax(arr, 5);
            execvp(instruc[max+1], &instruc[max+1]);
            fprintf(stderr,"%s: not a command\n",instruc[max+1]);      // if execvp cannot find the given command
            return -1;
        }
    } else 
    {
        wait(0);
    }

    return 0;
}