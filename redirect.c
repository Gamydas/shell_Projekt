#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "redirect.h"
#include "err.h"

/// @brief           opens up filedesciptors for a given redirect object, closing needs
///                  to be handled by the calling function
/// @param direction what type of redirect is called
/// @param target    target of redirection
/// @return          returns 0 if redirection was succesful, -1 otherwise
int redirection(redirect *dir)
{
    ERR error = NO_ERROR;
    int *stream = &dir->stream;
    char *target = &dir->target[0];
    if (*target == NULL)
    {
        error = INVALID_ARGUMENT;
        printError(error, "redirect");
    }
    // checks which redirection was called and opens an approriate descriptor
    switch (dir->direction)
    {
    case REDIR_OUT_TRUNC:
        *stream = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        break;
    case REDIR_OUT_APPEND:
        *stream = open(target, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        break;
    case REDIR_ERR_TRUNC:
        *stream = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        break;
    case REDIR_ERR_APPEND:
        *stream = open(target, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        break;

    case REDIR_IN:
        *stream = open(target, O_RDONLY);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        break;
    default:
        return -1; // should never be reached
    }

    return 0;
}


/// @brief switches the redirection mode and checks for syntax or logic errors
/// @param direction redirection mode
/// @param token given token, if its size is greater than 3 it is an automatic error
/// @param size length of token
/// @return 0 if succesful, -1 if an error occured
int switchDirect(REDIR *direction, char *token, int size)
{

    ERR error = NO_ERROR;
    if (size > 3)
    {
        error = SYNTAX_ERROR;
        // prints responsible char
        printError(error, &token[size - 1]);
        return -1; // no redirection operator is greater than 3 characters
    }

    if (size == 0)
    {
        *direction = REDIR_OUT_TRUNC;
        return 0;
    }

    if (size > 0)
    {

        switch (token[size - 1]) // always checking the prior operator
        {
        case '>':
            if (*direction == REDIR_OUT_TRUNC) // >>
            {
                *direction = REDIR_OUT_APPEND;
                return 0;
            }
            else if (*direction == REDIR_ERR_TRUNC) // 2>>
            {
                *direction = REDIR_ERR_APPEND;
                return 0;
            }
            else
            {
                error = SYNTAX_ERROR;
                // prints the character that caused the syntax error
                printError(error, &token[size - 1]);
                return -1;
            }
            break;
        case '2':
            *direction = REDIR_ERR_TRUNC; // 2>
            return 0;
        default:
            return -1; // this should never be reached
            break;
        }
    }
    return -1; // can never be reached, but makes the gcc happy :)
}