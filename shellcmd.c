#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "str.h"

void cd(char* dir)
{
    // if chdir failed i.e dir doesnt exist
    if(chdir(dir) == -1)
    {
        fprintf(stderr,"Directory does not exit.\n");
    }
}

void pwd()
{
    char* temp = malloc(200);
    printf("%s\n", getcwd(temp,200));
    free(temp);
}

void echo(char* text)
{
    printf("%s\n", text);
}

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