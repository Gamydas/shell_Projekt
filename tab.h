#include <dirent.h>
#ifndef TAB_H
#define TAB_H

typedef struct tabComp
{
    int tabs; // amount of tabs used
    int matchcount; // amount of matches found
    // structure to store all matches in, this needs to be large for completeExecs
    char** matches;
    int capac; // capacity of outer dimension, i.e the actual array
} tabComp;

int initTab(tabComp* tab);
int tabComplete(tabComp *tab, char (*builtins)[10], int binamt, char *command, char *path);
int completeBuiltins(tabComp *tab, char* token, char (*builtins)[10], int binamt);
int completeArgs(tabComp *tab, char *token, char *path, int isExec);
int completeExecs(tabComp *tab, char *token);
void handleCases(tabComp *tab, char* token);
void checkTabAmount(tabComp *tab);
void cleanupTab(tabComp *tab);

#endif