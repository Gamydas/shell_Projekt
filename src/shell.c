#include "shell.h"
#include "str.h"

/// @brief function initializes a shell struct
/// @param sh 
void init_shell(shell* sh)
{
   
    // history initializations
    sh->first_entry = NULL;
    sh->last_entry  = NULL;
    sh->current     = NULL;    

    // str and buffer initializations
    initialize_string(sh->wdir, 0, sizeof(sh->wdir));

    // initialize functiontable
    Builtin builtins[] = 
    {
        {"exit", shell_exit},
        {"cd", cd},
        {"type", type},
        {"pwd", pwd},
        //{"history", history},
        {NULL, NULL}
    };
    sh->bins = builtins;

    // initializing and populating hashtable
    hashmap_initialize(sh->builtins, 50);
    hashmap_populate(sh->builtins, 50, sh->bins);
    
    // termios initialization
    tcgetattr(0, &sh->canon);
    tcgetattr(0, &sh->raw);
    sh->raw.c_iflag &= ~(IXON);
    sh->raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

}