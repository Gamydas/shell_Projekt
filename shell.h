#include <termios.h>
#include <unistd.h>

#ifndef SHELL_H
#define SHELL_H

typedef struct shell
{
    char wdir[2048];                   // string for working directory 
    char hist[50][2048];                // array to store the command history in
    char builtins[50][10];             // an array to store the name of all builtins
    int binamt;                           // indicates the amount of builtins
    int histpos;                       // index for hist array
    
    
    struct termios canon;
    struct termios raw;
    
} shell;


void initShell(shell* sh);
#endif