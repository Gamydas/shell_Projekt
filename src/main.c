#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <termios.h>

#include "shell.h"
#include "input.h"
#include "builtins.h"
#include "escapesequenzen.h"
#include "parser.h"
#include "../lib/err.h"
#include "exec.h"

int main()
{
    ERR error = NO_ERROR;
    shell myShell;
    rawInput userInput;
    InstructList instructions;
    init_shell(&myShell);
    // initialize functiontable
    Builtin builtins[] = 
    {
        {"exit", shell_exit},
        {"cd", cd},
        {"type", type},
        {"pwd", pwd},
        {NULL, NULL}
    };
    // assign functiontable
    myShell.bins = builtins;
    

    CLEAR;
    HOME;
    while (1)
    {
        int control = initRaw(&userInput);
        if (control < 0)
        {
            error = INITIALIZATION_ERROR;
            printError(error, "initRaw");
            continue;
        }
        control = initInstructList(&instructions);
        if (control < 0)
        {
            error = INITIALIZATION_ERROR;
            printError(error, "initInstructList");
            freeRaw(&userInput); 
            continue;
        }
        // loading the current directory into it wdir string
        getcwd(myShell.wdir, sizeof(myShell.wdir));

        // loading the user command into cmd string
        control = getInput(&myShell, &userInput);
        if (control < 0)
        {
            freeRaw(&userInput);
            cleanupInstructList(&instructions);
            continue;
        }
        
        // seperating the command String into the seperate instructions
        control = parseInput(&instructions, userInput.cmd);
        if (control < 0)
        {
            freeRaw(&userInput);
            cleanupInstructList(&instructions);
            continue;
        }
        // is no longer needed for this loop
        freeRaw(&userInput);
        // this can happen in cases where user only types in ">"
        // or other characters that are filtered out by the parser
        if(instructions.size == 1) // only sentinel slot exists
        {
            error = SYNTAX_ERROR;
            printError(error, "newline");
            error = NO_ERROR;
            cleanupInstructList(&instructions);
            continue;
        }

        control = executeInstructs(&instructions, myShell.bins, myShell.binamt);
        cleanupInstructList(&instructions);
        if (control == 1) // exit was called
        {
            return 0; 
        }
    }
}
