#ifndef ERR_H
#define ERR_H

typedef enum
{
    NO_ERROR,
    SYNTAX_ERROR,
    INITIALIZATION_ERROR,
} ERR;

void printError(ERR error, char* context);
#endif
