#ifndef REDIRECT_H
#define REDIRECT_H

typedef enum
{
    NO_REDIR,          // neutral mode
    REDIR_OUT_TRUNC,   // stands for >
    REDIR_OUT_APPEND,  // stands for >>
    REDIR_ERR_TRUNC,   // stands for 2>
    REDIR_ERR_APPEND,  // stands for 2>>
    REDIR_IN           // stands for <
} REDIR;

typedef struct redirect
{
    REDIR direction; 
    int stream;
    char* target;
} redirect;

int switchDirect(REDIR *dir, char* token, int size);
int redirection(redirect *dir);







#endif