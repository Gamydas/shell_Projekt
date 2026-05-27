#ifndef SHELL_H
#define SHELL_H
#include <termios.h>
#include <unistd.h>
#include "builtins.h"
#include "history.h"

typedef struct shell
{
    char wdir[2048];        // string for working directory
    char builtins[50][10];  // an array to store the name of all builtins
    int binamt;             // indicates the amount of builtins
    // first i.e oldest history entry
    shHist *first_entry;
    // last i.e newest history entry
    shHist *last_entry;
    // current history entry
    shHist *current;


    Builtin *bins;  // always make this 1 larger than binamt for Sentinel
    struct termios canon;
    struct termios raw;

} shell;

void init_shell(shell *sh);
#endif