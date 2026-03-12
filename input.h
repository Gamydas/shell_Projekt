#include <termios.h>
#include "shell.h"
#include "parser.h"
#ifndef INPUT_H
#define INPUT_H
int getInput(shell* sh, command* cmd_);
int handleArrows(shell* sh, command* cmd_);
void reposCurs(int cursoridx);
//int handlePipes(shell* sh);
#endif

