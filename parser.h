#ifndef PARSER_H
#define PARSER_H
#include "pipelining.h"
#include "redirect.h"

typedef enum
{
    SINGLE_QUOTES,
    DOUBLE_QUOTES,
    NORMAL,

} MODUS;


typedef struct parsedInput
{
    char** parsed;      // array in which parsed tokens will be saved
    int parseamt;       // current fill amount of parsed, also used to check memory size reallocations
    redirect redir[10]; // why would you ever need more than 10 in 1 cmd
    int rdrctns;        // amount of redirectons
    pipeline* pipes;    // stores nessecary information for pipelining
    int pipecalls;      // amount of pipecalls, not an index so -1 if u want to use it as that
} command;

int parseInput(command* cmd_, char* text);
void switchModes(MODUS *mode, char c);
int initCMD(command* cmd_);
void cleanupCMD(command* cmd_);
#endif