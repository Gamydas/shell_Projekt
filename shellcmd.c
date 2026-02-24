#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void cd(char* dir)
{
    // if chdir failed i.e dir doesnt exist
    if(chdir(dir) == -1)
    {
        printf("Directory does not exit.\n");
    }
}

void pwd()
{
    char* temp = malloc(200);
    printf("%s\n", getcwd(temp,200));
    free(temp);
}