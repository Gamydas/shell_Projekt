#include <termios.h>
#include <unistd.h>

#ifndef SHELL_H
#define SHELL_H

typedef struct shell
{
    char wdir[2048];                   // string for working directory 
    char cmd[200];                     // string for user command
    char* instruc[50];                 // array of strings for seperate instructions within the command
    char buffer[50][200];               // memory to assign to instruc, might be changed for dynamic memory in the future
    char hist[50][200];                // array to store the command history in
    char builtins[50][10];             // an array to store the name of all builtins
    int binamt;                           // indicates the amount of builtins
    int binflag;                       // signals if a bin was found
    int exec;                          // -1 if no exec was used, 0 if exec was used
    int histpos;                       // index for hist array
    int cursoridx;                     // position index for the cursor on the screen
    
    
    
    struct termios canon;
    struct termios raw;
    
} shell;


void initShell(shell* sh);
#endif