#include <termios.h>
#include <unistd.h>
#ifndef SHELL_H
#define SHELL_H

typedef struct
{
    // dim 1 is the index dim 2 is the saved string
    char buf[50][50];
    int head;
    int tail;
} charCircBuff;


typedef struct
{
    char wdir[2048];                   // string for working directory 
    char cmd[200];                     // string for user command
    char* instruc[50];                 // array of strings for seperate instructions within the command
    char buffer[50][50];               // memory to assign to instruc, might be changed for dynamic memory in the future
    charCircBuff history;
    struct termios canon;
    struct termios raw;

} shell;

#endif