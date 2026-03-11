#include <dirent.h>
#include "shell.h"
#ifndef TAB_H
#define TAB_H

typedef struct tabComp
{
    int tabs; // amount of tabs used
    int matchcount; // amount of matches found
    // structure to store all matches in, this needs to be large for completeExecs
    char matches[400][2048];
} tabComp;

void initTab(tabComp* tab);
int checkPurpose(shell* sh);
void tabComplete(tabComp *tab, char (*builtins)[10], char *command, char *path);
void completeBuiltins(tabComp *tab, char* token, char (*builtins)[10]);
void completeArgs(tabComp *tab, char *token, char *path);
void completeExecs(tabComp *tab, char *token);
void handleCases(tabComp *tab, char* token);

#endif