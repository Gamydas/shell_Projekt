#ifndef SHELL_H
#define SHELL_H
#include <termios.h>
#include <unistd.h>
#include "builtins.h"
#include "history.h"

typedef struct shell
{
    char wdir[2048];        // string for working directory
    // first i.e oldest history entry
    shHist *first_entry;
    // last i.e newest history entry
    shHist *last_entry;
    // current history entry
    shHist *current;


    // did some testing, 50 as size of map avoids any collisions,
    // however should colissions arise with more builtins, simple increase size
    bin_Hashmap builtins[50];
    Builtin *bins;  
    struct termios canon;
    struct termios raw;

} shell;

void init_shell(shell *sh);
#endif