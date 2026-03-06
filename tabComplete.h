#include <dirent.h>
#ifndef TABCOMPLETE_H
#define TABCOMPLETE_H
typedef struct 
{
    DIR* cwd;
    struct dirent* compDir;
    char tempdir[256];
    char prefix[256];
    char* parse[200];
    char buffer[50][200];
    char twins[50][256];
    int count;
    int twidx;
    int args;
    int curlen;

} tabComp;

typedef struct shell shell;

void initTab(tabComp* tab);
void tabComplete(shell* sh);
void completeBuiltin(shell* sh); 
void completeArguments(shell *sh);


#endif 