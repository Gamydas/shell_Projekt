#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "input.h"
#include "str.h"

int main(int argc, char *argv[])
{
    /*printf("You have entered %d arguments:\n", argc);

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    */
    while(1)
    {
        // initializing a char array and loading the current directory into it
        char wdir[2048]; 
        getcwd(wdir, sizeof(wdir));

        // char array for the user command
        char cmd[200];               
        getInput(wdir,cmd);

        // exits the main loop if command is given
        if(strcomp(cmd,"exit")){
            return 0;
        }
    }
    return 0;
}