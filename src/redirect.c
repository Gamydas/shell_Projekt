#include "redirect.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lib/err.h"
#include "str.h"

/// @brief  opens up filedesciptors for a given redirect object, then dup2s
///         them to the desired channel and closes the fd, no file leaks
///         can happen here
/// @param direction what type of redirect is called
/// @param target    target of redirection
/// @return          returns 0 if redirection was succesful, -1 otherwise
int handle_redirections(redirect *dir)
{
    ERR error = NO_ERROR;
    int *stream = &dir->stream;
    char *target = dir->target;
    int cntrl = 0;
    if (target == NULL)
    {
        error = INVALID_ARGUMENT;
        print_error(error, "redirect");
        return -1;
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
        // duplicates the new fd to designated channel and closes it after
        cntrl = dup2(dir->stream, STDOUT_FILENO);
        if (cntrl < 0)
        {
            perror("dup2");
            return -1;
        }
        close(dir->stream);

        break;
    case REDIR_OUT_APPEND:
        *stream = open(target, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        // duplicates the new fd to designated channel and closes it after
        cntrl = dup2(dir->stream, STDOUT_FILENO);
        if (cntrl < 0)
        {
            perror("dup2");
            return -1;
        }
        close(dir->stream);

        break;
    case REDIR_ERR_TRUNC:
        *stream = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        // duplicates the new fd to designated channel and closes it after
        cntrl = dup2(dir->stream, STDERR_FILENO);
        if (cntrl < 0)
        {
            perror("dup2");
            return -1;
        }
        close(dir->stream);

        break;
    case REDIR_ERR_APPEND:
        *stream = open(target, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        // duplicates the new fd to designated channel and closes it after
        cntrl = dup2(dir->stream, STDERR_FILENO);
        if (cntrl < 0)
        {
            perror("dup2");
            return -1;
        }
        close(dir->stream);

        break;

    case REDIR_IN:
        *stream = open(target, O_RDONLY);
        if (*stream == -1)
        {
            perror("open");
            return -1;
        }
        // duplicates the new fd to designated channel and closes it after
        cntrl = dup2(dir->stream, STDIN_FILENO);
        if (cntrl < 0)
        {
            perror("dup2");
            return -1;
        }
        close(dir->stream);

        break;
    default:
        return -1;  // should never be reached
    }

    return 0;
}

/// @brief switches the redirection mode and checks for syntax or logic errors
/// @param direction redirection mode
/// @param token given token, if its size is greater than 3 it is an automatic error
/// @param size length of token
/// @return 0 if succesful, -1 if an error occured
int switch_directions(REDIR *direction, char *token, int size)
{
    ERR error = NO_ERROR;
    if (size > 3)
    {
        error = SYNTAX_ERROR;
        // prints responsible char
        print_error(error, &token[size - 1]);
        return -1;  // no redirection operator is greater than 3 characters
    }

    if (size == 0)
    {
        *direction = REDIR_OUT_TRUNC;
        return 0;
    }

    if (size > 0)
    {
        switch (token[size - 1])  // always checking the prior operator
        {
        case '>':
            if (*direction == REDIR_OUT_TRUNC)  // >>
            {
                *direction = REDIR_OUT_APPEND;
                return 0;
            }
            else if (*direction == REDIR_ERR_TRUNC)  // 2>>
            {
                *direction = REDIR_ERR_APPEND;
                return 0;
            }
            else
            {
                error = SYNTAX_ERROR;
                // prints the character that caused the syntax error
                print_error(error, &token[size - 1]);
                return -1;
            }
            break;
        case '2':
            *direction = REDIR_ERR_TRUNC;  // 2>
            return 0;
        default:
            return -1;  // this should never be reached
            break;
        }
    }
    return -1;  // can never be reached, but makes the gcc happy :)
}

int setup_redirections(redirect *redir, char *target, REDIR *type)
{
    redir->direction = *type;
    redir->target = calloc(1, str_len(target) + 1);  // +1 for \0
    if (redir->target == NULL)
    {
        perror("malloc");
        return -1;
    }
    str_copy(target, redir->target);
    return 0;
}