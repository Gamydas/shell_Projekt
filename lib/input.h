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

int initialize_rawinput(rawInput *input);
void free_rawinput(rawInput *input);
int get_input(shell* sh, rawInput* cmd_);
int handle_arrows(shell* sh, rawInput* cmd_);
void reposition_cursor(int cursoridx);
#endif

