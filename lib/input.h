#ifndef INPUT_H
#define INPUT_H
#include "shell.h"
#include "parser.h"

/// @brief this structs purpose is to store the raw data which the user inputs
///        for further processing by the parsing logic
typedef struct rawInput
{
    char* cmd;     // string to save raw user input
    int capac;     // capacity of input, will be doubled whenever limit is reached
    int cursoridx;  // value to store current cursorpos on the screen

} rawInput;

int initRaw(rawInput *input);
void freeRaw(rawInput *input);
int getInput(shell* sh, rawInput* cmd_);
int handleArrows(shell* sh, rawInput* cmd_);
void reposCurs(int cursoridx);
#endif

