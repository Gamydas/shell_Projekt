#include <stdlib.h>
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
    sh->bins = malloc(6 * sizeof(Builtin));
    // ugly boilerplate but easiest way to do this
    sh->bins[0].name = "exit";    sh->bins[0].bin = shell_exit;
    sh->bins[1].name = "cd";      sh->bins[1].bin = cd;
    sh->bins[2].name = "type";    sh->bins[2].bin = type;
    sh->bins[3].name = "pwd";     sh->bins[3].bin = pwd;
    sh->bins[4].name = "history"; sh->bins[4].bin = history;
    sh->bins[5].name = NULL;      sh->bins[5].bin = NULL;
    

    
    // initializing and populating hashtable
    hashmap_initialize(sh->builtins, 50);
    hashmap_populate(sh->builtins, 50, sh->bins);
    
    // termios initialization
    tcgetattr(0, &sh->canon);
    tcgetattr(0, &sh->raw);
    sh->raw.c_iflag &= ~(IXON);
    sh->raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

}