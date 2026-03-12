#include <stdio.h>
#include "err.h"

/// @brief prints out different error messages, depending on passed error
/// @param error error object passed to indicate which case happend
/// @param context certain errors require context, like syntax errors, if no context is needed pass NULL
void printError(ERR error, char* context)
{
    switch (error)
    {
    case SYNTAX_ERROR:
        fprintf(stderr, "syntax error near: '%s'\n", context);
        break;
    case INITIALIZATION_ERROR:
        fprintf(stderr, "Initiliation error at: %s\n", context);
    default:
        break;
    }
}