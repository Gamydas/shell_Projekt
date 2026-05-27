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
#include "history.h"

extern shHist *first_entry;
extern shHist *last_entry;
int main()
{
    ERR error = NO_ERROR;
    shell myShell;
    rawInput userInput;
    InstructList instructions;
    init_shell(&myShell);
    read_history_from_file();
    if(last_entry != NULL) myShell.histpos = last_entry->entry_ID;
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
        int control = initialize_rawinput(&userInput);
        if (control < 0)
        {
            error = INITIALIZATION_ERROR;
            print_error(error, "initialize_rawinput");
            continue;
        }
        control = initialize_instruct_list(&instructions);
        if (control < 0)
        {
            error = INITIALIZATION_ERROR;
            print_error(error, "initialize_instruct_list");
            free_rawinput(&userInput); 
            continue;
        }
        // loading the current directory into it wdir string
        getcwd(myShell.wdir, sizeof(myShell.wdir));

        // loading the user command into cmd string
        control = get_input(&myShell, &userInput);
        if (control < 0)
        {
            free_rawinput(&userInput);
            cleanup_instruct_list(&instructions);
            continue;
        }
        
        // seperating the command String into the seperate instructions
        control = parse_input(&instructions, userInput.cmd);
        if (control < 0)
        {
            free_rawinput(&userInput);
            cleanup_instruct_list(&instructions);
            continue;
        }
        // is no longer needed for this loop
        free_rawinput(&userInput);
        // this can happen in cases where user only types in ">"
        // or other characters that are filtered out by the parser
        if(instructions.size == 1) // only sentinel slot exists
        {
            error = SYNTAX_ERROR;
            print_error(error, "newline");
            error = NO_ERROR;
            cleanup_instruct_list(&instructions);
            continue;
        }

        control = setup_command_execution(&instructions, myShell.bins, myShell.binamt);
        cleanup_instruct_list(&instructions);
        // this should never be reached since exit is handled by setup_command_execution
        if (control == 1)
        {
            return 0; 
        }
    }
}
