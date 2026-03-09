#include <dirent.h>
#include "shell.h"
#ifndef TAB_H
#define TAB_H

typedef struct tabComp
{
    int tabs; // amount of tabs used
    int matchcount; // amount of matches found
    char matches[50][2048]; // structure to store all matches in
} tabComp;

void initTab(tabComp* tab);
int checkPurpose(shell* sh);
void tabComplete(tabComp *tab, char *command, char *path);
void completeArgs(tabComp *tab, char *token, char *path);
void handleCases(tabComp *tab, char* token);

#endif