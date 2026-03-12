#ifndef PARSER_H
#define PARSER_H

typedef enum
{
    SINGLE_QUOTES,
    DOUBLE_QUOTES,
    NORMAL,

} MODUS;

typedef enum
{
    NO_REDIR,          // neutral mode
    REDIR_OUT_TRUNC,   // stands for >
    REDIR_OUT_APPEND,  // stands for >>
    REDIR_ERR_TRUNC,   // stands for 2>
    REDIR_ERR_APPEND,  // stands for 2>>
    REDIR_IN           // stands for <
} REDIR;

typedef struct redirect
{
    REDIR direction; 
    int stream;
    char* target;
} redirect;

typedef struct command
{
    char* cmd;
    int capac;          // capacity of cmd string, i.e the allocated size 
    char** parsed;      // array in which parsed tokens will be saved
    int parseamt;       // current fill amount of parsed, also used to check memory size reallocations
    int cursoridx;      // position of cursor inside of the string
    redirect redir[10]; // why would you ever need more than 10 in 1 cmd
    int rdrctns;        // amount of redirectons
} command;

int parseInput(command* cmd_);
void switchModes(MODUS *mode, char c);
int switchDirect(REDIR *dir, char* token, int size);
int initCMD(command* cmd_);
void cleanupCMD(command* cmd_);
#endif