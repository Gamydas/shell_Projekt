#ifndef EXEC_H
#define EXEC_H
#include "parser.h"
#include "builtins.h"


int setup_command_execution(InstructList* list, shell *sh);
int execute_commands(Instructions *instruts, shell *sh, int ID);
#endif