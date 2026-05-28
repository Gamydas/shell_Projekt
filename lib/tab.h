#include <dirent.h>
#ifndef TAB_H
#define TAB_H

typedef struct Builtin Builtin;
typedef struct tabComp
{
    int tabs; // amount of tabs used
    int matchcount; // amount of matches found
    // structure to store all matches in, this needs to be large for complete_executables
    char** matches;
    int capac; // capacity of outer dimension, i.e the actual array
} tabComp;

int initialize_tab_struct(tabComp* tab);
int tab_completion(tabComp *tab, Builtin *table, char *command, char *path);
int complete_builtins(tabComp *tab, char* token, Builtin* table);
int complete_arguments(tabComp *tab, char *token, char *path, int isExec);
int complete_executables(tabComp *tab, char *token);
void handle_tab_cases(tabComp *tab, char* token);
void check_tab_amount(tabComp *tab);
void cleanup_tab_struct(tabComp *tab);

#endif