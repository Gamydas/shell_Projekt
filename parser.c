#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"
#include "err.h"
#include "str.h"

#define PATH_MAX 4096 // my system doesnt find PATH_MAX in limits.h itll be defined locally here

/// @brief           opens up filedesciptors for a given redirect object, closing needs
///                  to be handled by the calling function
/// @param direction what type of redirect is called
/// @param target    target of redirection
/// @return          returns 0 if redirection was succesful, -1 otherwise
int redirection(redirect *dir)
{

    int *stream = &dir->stream;
    char *target = &dir->target[0];

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

/// @brief This function sets the current working mode of the parser.
///        Single and Double respond to the respective character, every
///        other read character will not affect the modes.
///        In Normal mode every character is taken processed their intended
///        purpose. More documentation on special characters follows
/// @param mode
/// @param c
void switchModes(MODUS *mode, char c)
{
    switch (c)
    {
    case '"':
        // switches to D_Q if current mode is normal
        if (*mode == NORMAL)
        {
            *mode = DOUBLE_QUOTES;

            break;
        }
        // if in D_Q switches back to normal (might need to look this over later)
        if (*mode == DOUBLE_QUOTES)
        {
            *mode = NORMAL;

            break;
        }
        break;

    case 39: // single quote '
        // switches to S_Q if current mode is normal
        if (*mode == NORMAL)
        {
            *mode = SINGLE_QUOTES;

            break;
        }
        // if in S_Q switches back to normal (might need to look this over later)
        if (*mode == SINGLE_QUOTES)
        {
            *mode = NORMAL;

            break;
        }
        break;
    default:
        break;
    }
}

/// @brief this functions purpose is to parse a given string
///        according to a very specific set of rules, rules will follow
/// @param text the raw input string handed given by input module
/// @return returns 0 if succesful, -1 if not
int parseInput(command *cmd_, char* text)
{
    ERR error = NO_ERROR;
    MODUS mode = NORMAL;
    REDIR type = NO_REDIR;
    int *parseamt = &cmd_->parseamt; // amount of parsed tokens; the first dimension of a char[][]
    int idx = 0;                     // length of currently parsed token; second dimension of char[][]
    int rdrct = 0;                   // if this is 0 no redirection was called, 1 if one was called
    char token[PATH_MAX];            // a buffer to temporarily hold the token that is to be parsed

    while (*text)
    {
        switch (*text)
        {
        case '\t':
            // redirection has been called
            if (type != NO_REDIR)
            {
                rdrct++;
            }

            // this avoids token creation for redirection operator
            if (rdrct == 1)
            {
                text++;
                idx = 0;
                break;
            }

            // filters tabs at beginning of command
            if (*parseamt == 0 && idx == 0 && mode == NORMAL)
            {
                while (*text == '\t')
                    text++;
                break;
            }

            // new token
            if (mode == NORMAL)
            {
                // this checks for consequtive seperators to avoid false tokens
                if (*(text - 1) == '\t')
                {
                    text++;
                    break;
                }
                token[idx] = '\0'; // terminates token

                /* if rdrct is 2 then 2 seperators have been called since the
                   the last redirection operator, meaning this is the target
                   token*/
                if (rdrct == 2)
                {
                    cmd_->redir[cmd_->rdrctns].direction = type;
                    cmd_->redir[cmd_->rdrctns].target = malloc(strLen(token) + 1); // +1 for \0
                    if (cmd_->redir[cmd_->rdrctns].target == NULL)
                    {
                        perror("malloc");
                        return -1;
                    }
                    strcopy(token, cmd_->redir[cmd_->rdrctns].target);
                    /* checks if redirection was a success and returns
                       an erro if it wasnt, error message handled by function */
                    if (redirection(&cmd_->redir[cmd_->rdrctns]) == 0)
                    {
                        cmd_->rdrctns++;
                    }
                    else
                    {
                        return -1;
                    }

                    rdrct = 0;
                    type = NO_REDIR;
                    text++;
                    idx = 0;
                    break;
                }
                // this checks if the max size of the array was reached and doubles it if so
                if (*parseamt > 0 && *parseamt % 30 == 0)
                {
                    char **temp = realloc(cmd_->parsed, (*parseamt * 2) * sizeof(char *));
                    if (temp == NULL)
                    {
                        perror("malloc");
                        return -1;
                    }
                    cmd_->parsed = temp;
                }
                // transfers the token into the parsed arr of the cmd struct
                cmd_->parsed[*parseamt] = malloc(strLen(token) + 1); // +1 for \0
                if (cmd_->parsed[*parseamt] == NULL)
                {
                    perror("malloc");
                    return -1;
                }
                strcopy(token, cmd_->parsed[*parseamt]);
                (*parseamt)++; // opens new token
                text++;
                idx = 0; // resets length for new token
                break;
            }
            // space gets read as a normal sign, so it goes into the default case
            if (mode == SINGLE_QUOTES || mode == DOUBLE_QUOTES)
            {
                goto append;
            }
            break;
        case 32: // space
            // redirection has been called
            if (type != NO_REDIR)
            {
                rdrct++;
            }
            // this avoids token creation for redirection operator
            if (rdrct == 1)
            {
                text++;
                idx = 0;
                break;
            }
            // filters spaces at beginning of command
            if (*parseamt == 0 && idx == 0 && mode == NORMAL)
            {
                while (*text == 32)
                    text++;
                break;
            }
            // new token
            if (mode == NORMAL)
            {
                // this checks for consequtive seperators to avoid false tokens
                if (*(text - 1) == 32)
                {
                    text++;
                    break;
                }
                token[idx] = '\0'; // terminates token
                /* if rdrct is 2 then 2 seperators have been called since the
                   the last redirection operator, meaning this is the target
                   token*/
                if (rdrct == 2)
                {
                    cmd_->redir[cmd_->rdrctns].direction = type;
                    cmd_->redir[cmd_->rdrctns].target = malloc(strLen(token) + 1); // +1 for \0
                    if (cmd_->redir[cmd_->rdrctns].target == NULL)
                    {
                        perror("malloc");
                        return -1;
                    }
                    strcopy(token, cmd_->redir[cmd_->rdrctns].target);
                    /* checks if redirection is succesful and returns an error if not
                       errormessage gets sent by functions*/
                    if (redirection(&cmd_->redir[cmd_->rdrctns]) == 0)
                    {
                        cmd_->rdrctns++;
                    }
                    else
                    {
                        return -1;
                    }
                    rdrct = 0;
                    type = NO_REDIR;
                    // to avoid this token appearing as an argument in parsed
                    text++;
                    idx = 0;
                    break;
                }

                // this checks if the max size of the array was reached and doubles it if so
                if (*parseamt > 0 && *parseamt % 30 == 0)
                {
                    char **temp = realloc(cmd_->parsed, (*parseamt * 2) * sizeof(char *));
                    if (temp == NULL)
                    {
                        perror("malloc");
                        return -1;
                    }
                    cmd_->parsed = temp;
                }

                // transfers the token into the parsed arr of the cmd struct
                cmd_->parsed[*parseamt] = malloc(strLen(token) + 1); // +1 for \0
                if (cmd_->parsed[*parseamt] == NULL)
                {
                    perror("malloc");
                    return -1;
                }
                strcopy(token, cmd_->parsed[*parseamt]);

                (*parseamt)++; // opens new token
                text++;
                idx = 0; // resets length for new token
                break;
            }
            // space gets read as a normal sign, so it goes into the default case
            if (mode == SINGLE_QUOTES || mode == DOUBLE_QUOTES)
            {
                goto append;
            }
            break;
        case '"':
            switchModes(&mode, *text);
            text++; // this skips the quotes so they dont land in the token
            break;
        case 39: // '
            switchModes(&mode, *text);
            text++; // this skips the quotes so they dont land in the token
            break;
        case '>':
            if (mode == SINGLE_QUOTES || mode == DOUBLE_QUOTES)
            {
                goto append;
            }
            if (switchDirect(&type, token, idx) == 0)
            {
                text++;
                idx++;
            }
            else
            {
                return -1; // syntax error, handled by switchDirect
            }
            break;

        case '<':
            if (mode == SINGLE_QUOTES || mode == DOUBLE_QUOTES)
            {
                goto append;
            }
            if (idx > 0) // < can only stand alone
            {
                error = SYNTAX_ERROR;
                printError(error, text - 1);

                /* in case of an error during the setup of filedescriptors
                    this closes every previously opend one and returns -1*/
                for (int i = 0; i < cmd_->rdrctns; i++)
                {
                    close(cmd_->redir[i].stream);
                }
                return -1;
            }
            type = REDIR_IN;
            text++;
            idx++;
            break;
            /*
            case '$':



            case  39: // single quote

            case '*':

            case '?':

            case '~':
            */

        default: // default case simply appends the current character to the current token
        append:  // C doesnt allow goto default; so this is the fallthrough point for most functions
            token[idx] = *text;
            idx++;
            text++;
            if (!*text)
            {
                token[idx] = '\0'; // terminates final token
                // this checks if the last token is a redirect target
                if (rdrct == 1)
                {
                    cmd_->redir[cmd_->rdrctns].direction = type;
                    cmd_->redir[cmd_->rdrctns].target = malloc(strLen(token) + 1); // +1 for \0
                    if (cmd_->redir[cmd_->rdrctns].target == NULL)
                    {
                        perror("malloc");
                        return -1;
                    }
                    strcopy(token, cmd_->redir[cmd_->rdrctns].target);
                    /* checks if redirection is succesful and returns an error if not
                       errormessage gets sent by functions*/
                    if (redirection(&cmd_->redir[cmd_->rdrctns]) == 0)
                    {
                        cmd_->rdrctns++;
                    }
                    else
                    {
                        return -1;
                    }
                    // array termination for exec and co. 
                    cmd_->parsed[*parseamt] = NULL; 
                    return 0;
                }

                // this checks if the max size of the array was reached and doubles it if so
                if (*parseamt > 0 && *parseamt % 30 == 0)
                {
                    char **temp = realloc(cmd_->parsed, (*parseamt * 2) * sizeof(char *));
                    if (temp == NULL)
                    {
                        perror("malloc");
                        return -1;
                    }
                    cmd_->parsed = temp;
                }
                cmd_->parsed[*parseamt] = malloc(strLen(token) + 1); // +1 for \0
                if (cmd_->parsed[*parseamt] == NULL)
                {
                    perror("malloc");
                    return -1;
                }
                strcopy(token, cmd_->parsed[*parseamt]);
            }
            break;
        }
    }

    (*parseamt)++;
    cmd_->parsed[*parseamt] = NULL; // array termination
    return 0;
}

/// @brief
/// @param cmd
/// @return returns 0 if successful, -1 if not
int initCMD(command *cmd_)
{
    // initializes all struct variables
    for (int i = 0; i < 10; i++)
    {
        cmd_->redir[i].direction = NO_REDIR;
        cmd_->redir[i].stream = -1;
        cmd_->redir[i].target = NULL;
    }
    
    cmd_->parseamt = 0;
    cmd_->rdrctns = 0;
    cmd_->parsed = NULL;
    cmd_->parsed = calloc(30, sizeof(char *)); // base length, gets doubled if nessecary
    if (cmd_->parsed == NULL)
    {
        perror("malloc");
        return -1;
    }

    return 0;
}

/// @brief frees allocated memory of a command struct
/// @param cmd_
void cleanupCMD(command *cmd_)
{
    for (int i = 0; i < cmd_->parseamt; i++)
    {
        free(cmd_->parsed[i]);
    }
    free(cmd_->parsed);
    /* in case of an error during the setup of filedescriptors
        this closes every previously opend one and returns -1*/
    for (int i = 0; i < cmd_->rdrctns; i++)
    {
        free(cmd_->redir[i].target);
        close(cmd_->redir[i].stream);
    }
}