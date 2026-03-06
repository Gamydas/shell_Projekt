#include <termios.h>
#include "shell.h"
#ifndef INPUT_H
#define INPUT_H
void getInput(shell* sh);
void handleArrows(shell* sh);
void reposCurs(shell* sh);
int handlePipes(shell* sh);
int redirect(shell* sh);
#endif

