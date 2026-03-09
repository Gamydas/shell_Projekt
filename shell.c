#include "shell.h"
#include "str.h"
#include "tabComplete.h"

/// @brief function initializes a shell struct
/// @param sh 
void initShell(shell* sh)
{
    // integer intitializations
    sh->cursoridx = 0;
    sh->histpos = 0;
    sh->binamt = 4;
    sh->binflag = -1;

    
    // str and buffer initializations
    initStr(sh->cmd, 0, sizeof(sh->cmd));
    initStr(sh->wdir, 0, sizeof(sh->wdir));
    for (int i = 0; i < 50; i++)
    {
        initStr(sh->buffer[i], 0, sizeof(sh->buffer[i]));
        initStr(sh->hist[i], 0, sizeof(sh->hist[i]));
        initStr(sh->builtins[i],0, sizeof(sh->builtins[0]));
    }

    // entering all currently existing builtins into the array
    strcopy("cd", sh->builtins[0]);
    strcopy("pwd", sh->builtins[1]);
    strcopy("echo", sh->builtins[2]);
    strcopy("type", sh->builtins[3]);
    
    // termios initialization
    tcgetattr(0, &sh->canon);
    tcgetattr(0, &sh->raw);
    sh->raw.c_iflag &= ~(IXON);
    sh->raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

}