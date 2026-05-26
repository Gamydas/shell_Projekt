#ifndef SHELL_H
#define SHELL_H
#include <termios.h>
#include <unistd.h>
#include "builtins.h"
typedef struct shell
{
    char wdir[2048];                   // string for working directory 
    char builtins[50][10];             // an array to store the name of all builtins
    int binamt;                           // indicates the amount of builtins
    int histpos;                       // index for hist array
    
    Builtin* bins;                   // always make this 1 larger than binamt for Sentinel
    struct termios canon;
    struct termios raw;
    
} shell;


void init_shell(shell* sh);
#endif