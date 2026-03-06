#include "tabComplete.h"

#include <stdio.h>

#include "shell.h"
#include "str.h"
#include "buildtins.h"

void initTab(tabComp* tab)
{
    tab->cwd = NULL;
    tab->compDir = NULL;
    initStr(tab->tempdir, 0, 256);
    initStr(tab->prefix, 0, 256);
    for (int i = 0; i < 50; i++)
    {
        initStr(tab->twins[i], 0, sizeof(tab->twins[i]));
        initStr(tab->buffer[i], 0, sizeof(tab->buffer[i]));
    }
    for (int i = 0; i < 50; i++)
    {
        tab->parse[i] = tab->buffer[i];
    }

    
    tab->twidx = 0;
    tab->args = 0;
    tab->curlen = 0;
    tab->count = 0;
}

void tabCases(shell* sh)
{
    int length = strLen(sh->cmd);
    int idx = strLen(sh->compl.parse[sh->compl.args-1]); // gets the index of the to be completed item
    // no match was found
    if (sh->compl.twidx == 0)
    {
        printf("\a");
        fflush(stdout);
        return;
    }
    // exactly one match was found
    if (sh->compl.twidx == 1)
    {
        strcopy(sh->compl.twins[0], &sh->cmd[length-idx]);
        int temp = strLen(sh->compl.twins[0]) + 1;
        sh->cmd[temp] = 32;
        sh->cmd[temp + 1] = '\0';
        return;
    }
    // more than one match was found
    if (sh->compl.twidx > 1)
    {
        printf("\a");
        fflush(stdout);
        findPrefix(sh->compl.twins, sh->compl.prefix, 256, sh->compl.twidx);
        strcopy(sh->compl.prefix, &sh->cmd[length-idx]);
        return;
    }
}

/// @brief this function checks if the type of file that the current dir is matches the requirement for the command that was input
/// @param dir
/// @param flag pass the binflag here to tell the function which builtin was parsed
/// @return returns 0 if matching, -1 otherwise
int checkCompatibleFileTyping(struct dirent* dir, int flag)
{
    switch (flag)
    {
        case 0:  // cd
            if (dir->d_type == DT_DIR)
            {
                return 0;
            }
            return -1;

        case 1:  // pwd
            return -1;
        case 2:  // echo
            if (dir->d_type != DT_UNKNOWN)
            {
                return 0;
            }
            return -1;
        case 3:  // type
            // this might change later depending how much more detailed I make this shell in the future
            if (dir->d_type != DT_UNKNOWN)
            {
                return 0;
            }
            return -1;

        default:
            return -1;
    }
}

void completeArguments(shell* sh)
{
    int idx = sh->compl.args - 1;  // Idx of the yet unwritten

    while((sh->compl.compDir = readdir(sh->compl.cwd)) != NULL)
    {
        // partial match found
        if(strcomp(sh->compl.parse[idx], sh->compl.compDir->d_name) == 1) 
        {
            // checks if the type of file is compatible with the command and adds it to twins if so
            if(checkCompatibleFileTyping(sh->compl.compDir, sh->binflag) == 0)
            {
                strcopy(sh->compl.compDir->d_name, sh->compl.twins[sh->compl.twidx]);
                sh->compl.twidx++;;
            }
        }
    }
    
    tabCases(sh);
}

void completeBuiltin(shell* sh)
{
    // checks if the command was already a full builtin or only a partial
    for (int i = 0; i < sh->binamt; i++)
    {
        if (strcomp(sh->builtins[i], sh->compl.parse[0]) == 0)
        {
            sh->binflag= i;
        }
    }

    /* if this is true the user typed in a full command, but not a
       single letter to complete via TAB so the bell rings
    */
    if (sh->binflag != -1)
    {
        sh->binflag = -1;
        printf("\a");
        fflush(stdout);
        return;
    }
    else
    {
        for (int i = 0; i < sh->binamt; i++)
        {
            if (strcomp(sh->compl.parse[0], sh->builtins[i]) == 1)
            {
                strcopy(sh->builtins[i], sh->compl.twins[sh->compl.twidx]);
                sh->binflag = i;
                sh->compl.twidx++;
            }
        } /* this block iterates over every item in sh->builtins and
             compares parse to it stringwise if a match has been
             found it gets written into twins and twidx gets incremented*/
    }

    tabCases(sh);
}

/// @brief support function which gets passed a shell struc and looks through present directories and looks for either complete or partial matches,
///        which will then be saved to the twins array(later the entire array will be printed out on a double press of TAB)
/// @param sh
void tabComplete(shell* sh)
{
    initTab(&sh->compl);
    sh->compl.cwd = opendir(sh->wdir);  // opens the directory stream of the current working directory
    sh->compl.args = parseStr(sh->cmd, sh->compl.parse);
    int args = sh->compl.args;
    if (args > 0)
    {
        int curlen = strLen(sh->compl.parse[args - 1]);

        // only a command(builtin or executable) has been (partially) typed
        if (args == 1)
        {
            completeBuiltin(sh);
        } else if (args > 1)
        {
            // gets bin flag to discern what type of completion is to be done 
            sh->binflag = fetchBinFlag(sh->compl.parse, args);
            completeArguments(sh);
        }
    }

    closedir(sh->compl.cwd);
}
