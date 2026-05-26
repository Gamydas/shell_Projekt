#ifndef EXEC_H
#define EXEC_H
#include "parser.h"
#include "builtins.h"

int setup_command_execution(InstructList* list, Builtin *builtins, int binamt);
int execute_commands(Instructions *instruts, Builtin *builtins, int binamt, int ID);
#endif