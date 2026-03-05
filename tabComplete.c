#include <stdio.h>
#include "tabComplete.h"
#include "shell.h"
#include "str.h"


void initTab(tabComp* tab)
{
    tab->cwd = NULL;
    tab->compDir = NULL;
    initStr(tab->tempdir,0,256);
    initStr(tab->prefix,0,256);
    for (int i = 0; i < 50; i++)
    {
        initStr(tab->twins[i], 0, sizeof(tab->twins[i]));
        initStr(tab->buffer[i], 0, sizeof(tab->buffer[i]));
    }
    for (int i = 0; i < 50; i++)
    {
        tab->parse[i] = tab->buffer[i];
    }
    
    tab->binflag = -1;
    tab->twidx = 0;
    tab->args = 0;
    tab->curlen = 0;
    tab->count = 0;
}

