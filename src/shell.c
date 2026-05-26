#include "shell.h"
#include "str.h"

/// @brief function initializes a shell struct
/// @param sh 
void init_shell(shell* sh)
{
    // integer intitializations
    sh->histpos = 0;
    sh->binamt = 4;
    
    // str and buffer initializations
    initStr(sh->wdir, 0, sizeof(sh->wdir));
    for (int i = 0; i < 50; i++)
    {
        initStr(sh->builtins[i],0, sizeof(sh->builtins[0]));
    }

    // entering all currently existing builtins into the array
    strcopy("exit", sh->builtins[0]);
    strcopy("cd", sh->builtins[1]);
    strcopy("type", sh->builtins[2]);
    strcopy("pwd", sh->builtins[3]);
    
    // termios initialization
    tcgetattr(0, &sh->canon);
    tcgetattr(0, &sh->raw);
    sh->raw.c_iflag &= ~(IXON);
    sh->raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

}