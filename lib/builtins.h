#ifndef BUILTINS_H
#define BUILTINS_H

typedef void (*BinFn) (char*); // function pointer to a char* taking function
// functiontable, for cleaner execution
typedef struct 
{
    char *name;
    BinFn bin;
    
} Builtin;

void shell_exit(char* dir);
void cd(char* dir);
void pwd(char* flags);
void type(char* text);
void echo(char* text);
#endif