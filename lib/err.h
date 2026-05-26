#ifndef ERR_H
#define ERR_H

typedef enum
{
    NO_ERROR,
    SYNTAX_ERROR,
    INITIALIZATION_ERROR,
    INVALID_ARGUMENT
} ERR;

void print_error(ERR error, char* context);
#endif
