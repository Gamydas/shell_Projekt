#include <termios.h>
#include "shell.h"
#ifndef INPUT_H
#define INPUT_H
void getInput(shell* sh);
void tabComplete(shell* sh);
#endif

