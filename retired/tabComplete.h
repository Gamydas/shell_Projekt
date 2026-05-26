#include <dirent.h>
#ifndef tab_completion_H
#define tab_completion_H
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

void initialize_tab_struct(tabComp* tab);
void tab_completion(shell* sh);
void completeBuiltin(shell* sh); 
void completeArguments(shell *sh);


#endif 