#include "shell.h"
#include "str.h"

/// @brief function initializes a shell struct
/// @param sh 
void initShell(shell* sh)
{
    sh->latest = 0;
    sh->cursoridx = 0;
    sh->cmd[0] = '\0';
    sh->in_history = 0;
    sh->doubletab = 0;
    sh->histpos = 0;
    for (int i = 0; i < 50; i++)
    {
        initStr(sh->hist[i], 0, sizeof(sh->hist[i]));
    }
    
    tcgetattr(0, &sh->canon);
    tcgetattr(0, &sh->raw);
    sh->raw.c_iflag &= ~(IXON);
    sh->raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

}