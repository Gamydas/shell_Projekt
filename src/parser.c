#include "parser.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "err.h"
#include "redirect.h"
#include "str.h"

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

    case 39:  // single quote '
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

/// @brief adds a new item to the list of instructions and ensures connects and instructs dont drift apart in size
/// @param list
/// @return
int addSegment(InstructList *list, Instructions *instructs, CONNECTOR connect)
{
    list->size++;  // first increase size
    Instructions *temp = realloc(list->instructs, list->size * sizeof(Instructions));
    if (temp == NULL)
    {
        perror("malloc");
        return -1;
    }
    list->instructs = temp;
    CONNECTOR *temp2 = realloc(list->connects, list->size * sizeof(CONNECTOR));
    if (temp2 == NULL)
    {
        // dont free temp here, the user of InstructList is responsible for cleaning up
        perror("malloc");
        return -1;
    }
    list->connects = temp2;

    // always adds the new items to the second to last slot, last slot always reserved for Sentinel
    list->instructs[list->size - 2].args = instructs->args;
    list->instructs[list->size - 2].capac = instructs->capac;
    list->instructs[list->size - 2].parseamt = instructs->parseamt;
    list->instructs[list->size - 2].rdrctns = instructs->rdrctns;

    for (int i = 0; i < instructs->rdrctns; i++)
    {
        list->instructs[list->size - 2].redir[i] = instructs->redir[i];
    }

    list->connects[list->size - 2] = connect;
    list->instructs[list->size - 1].args = NULL;
    list->connects[list->size - 1] = NO_CONNECTOR;
    return 0;
}

int handleConnectors(Instructions *instructs, InstructList *list, CONNECTOR connect)
{
    instructs->parseamt++;                        // parseamt can never reach capac before this, so no realloc check needed, just place sentinel
    instructs->args[instructs->parseamt] = NULL;  // NULL sentinel
    int cntrl = addSegment(list, instructs, connect);
    if (cntrl < 0)
    {
        // error message thrown by exited funtion
        return -1;  // caller responsible for cleanup
    }
    if (connect != END)
    {
        cntrl = initInstructs(instructs);  // resets instruction object and creates new memory adresses, i.e a totally new item
        if (cntrl < 0)
        {
            // error message thrown by exited funtion
            return -1;  // caller responsible fo
        }
    }

    return 0;
}

/// @brief filters every seperator at the beginning of the given token, then checks if
///        every nessecary case
/// @param instruct
/// @param token
/// @param text
/// @param idx position in token
/// @param pos position in text
/// @param type
/// @return 0 if success, -1 if error
int handleSeperator(Instructions *instruct, char *token, char *text, int *idx, int *pos, REDIR *type)
{
    int cntrl = 0;

    // clears out all sequential seperators to avoid false tokenization
    while (text[*pos] == 32 || text[*pos] == '\t')
    {
        (*pos)++;
    }
    // checks if the found seperator was at the beginning of a token
    if (*idx == 0 && instruct->parseamt == 0)
    {
        return 0;
    }
    // finishing token and resetting idx
    token[*idx] = '\0';
    *idx = 0;

    // checks if redirection targets need to be set
    if (*type != NO_REDIR && *type != END_NEXT)
    {
        instruct->redir[instruct->rdrctns].direction = *type;  // saves redirect type
        *type = END_NEXT;                                      // indicates that next token is target
        // Do not increase pos here, already happend in the first while, pos already points to the next tokens beginning
        // idx is already 0 so no need to reset it here
        initStr(token, 0, strLen(token));  // clears token to avoid wrong reads
        return 0;                          // need to return so the redir operator doesnt get tokenized
    }
    else if (*type == END_NEXT)
    {
        *type = instruct->redir[instruct->rdrctns].direction;  // fetches current redirection type
        cntrl = setUpRedir(&instruct->redir[instruct->rdrctns], token, type);
        if (cntrl < 0)
        {
            return -1;  // error occured, message handled by exited funct, cleanup handled by caller
        }
        instruct->rdrctns++;
        // Do not increase pos here, already happend in the first while, pos already points to the next tokens beginning
        // idx is already 0 so no need to reset it here
        initStr(token, 0, strLen(token));  // clears token to avoid wrong reads
        *type = NO_REDIR;                  // resets redirection type for next redir
        return 0;
    }

    // checks if new memory has to be allocated
    if (instruct->parseamt == instruct->capac - 1)  // always leave one space for sentinel
    {
        cntrl = increaseCapac(&instruct->args, &instruct->capac, instruct->capac);  // doubles capac
        if (cntrl < 0)
        {
            return -1;
        }
    }
    // adds the finished token to the arguments array
    cntrl = allocStrCopy(token, &instruct->args[instruct->parseamt], strLen(token));
    if (cntrl < 0)
    {
        return -1;
    }
    instruct->parseamt++;
    // do not increment pos here, that is already done in the first while
    return 0;
}

/// @brief adds text[pos] to token[idx] then increments both, if text[pos] is \0 finishes the last token
/// @param cmd_
/// @param token token to which the given character is to be appended
/// @param text pointer to a string, neessecary because it is to be changed outside of this functions scope
/// @param idx  current idex of token
/// @param pos position in text
/// @return
int appendToToken(Instructions *instruct, char *token, char *text, int *idx, int *pos, REDIR type)
{
    int cntrl = 0;
    if (text[*pos])
    {
        token[*idx] = text[*pos];
        (*idx)++;
        (*pos)++;
        return 0;
    }else if (!text[*pos]) // This got seperated so there is one total final call of this function 
    {                      // in parser and not a bunch of different if(!text[pos]) cases
        token[*idx] = '\0';  // terminates final token
        // this checks if the last token is a redirect target
        if (type == END_NEXT)
        {
            type = instruct->redir[instruct->rdrctns].direction;  // fetches the current redirection type
            cntrl = setUpRedir(&instruct->redir[instruct->rdrctns], token, &type);
            if (cntrl < 0)
            {
                return -1;  // same error handling as everywhere else in this module
            }
            instruct->rdrctns++;
            instruct->args[instruct->parseamt] = NULL;  // redir target is not to be tokenized!
            return 0;
        }

        // checks if more memory needs to be allocated
        if (instruct->parseamt == instruct->capac - 1)
        {
            cntrl = increaseCapac(&instruct->args, &instruct->capac, 1);  // only needs 1 more slot for the NULL sentinel
            if (cntrl < 0)
            {
                return -1;  // same error handling as everywhere else in this module
            }
        }
        // enters token into the parsed arrays
        // transfers the token into the parsed arr of the cmd struct
        cntrl = allocStrCopy(token, &instruct->args[instruct->parseamt], strLen(token));
        if (cntrl < 0)
        {
            return -1;  // malloc error
        }
        instruct->parseamt++;
        instruct->args[instruct->parseamt] = NULL;  // Sentinel
    }
    return 0;
}

/// @brief parses the given string according to certain rules, will be documented seperately
/// @param list list item
/// @param text raw Input, usually handed by input.c module
/// @return 0 if success, -1 if error
int parseInput(InstructList *list, char *text)
{
    ERR error = NO_ERROR;
    MODUS mode = NORMAL;
    REDIR dir = NO_REDIR;
    int cntrl = 0;
    int idx = 0;
    int pos = 0;
    char token[PATH_MAX];
    Instructions instructs;
    initInstructs(&instructs);
    while (text[pos])
    {
        switch (text[pos])
        {
        case '\t':
            // INTENDED FALLTHROUGH! avoids double code
        case 32:  // space
            if (mode != NORMAL)
            {
                cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
                if (cntrl < 0)
                {
                    cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                    return -1;                     // error occured, message will be given by exited funct
                }
                break;
            }
            cntrl = handleSeperator(&instructs, token, text, &idx, &pos, &dir);
            if (cntrl < 0)
            {
                cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                return -1;                     // error occured, message will be given by exited funct
            }
            break;
        case '|':  // pipe
            if (mode != NORMAL)
            {
                cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
                if (cntrl < 0)
                {
                    cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                    return -1;                     // error occured, message will be given by exited funct
                }
                break;
            }
            // i.e this is the first token in an instruction, which is illegal
            if (instructs.parseamt == 0)
            {
                error = SYNTAX_ERROR;
                printError(error, &text[pos]);
                cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                return -1;
            }
            cntrl = handleConnectors(&instructs, list, PIPE);
            if (cntrl < 0)
            {
                // error message thrown by exited funtion
                cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                return -1;                     // caller responsible for cleanup
            }
            idx = 0;  // a connector is the end of a token
            pos++;    // move position
            break;

        case ';':  // seq operator
            if (mode != NORMAL)
            {
                cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
                if (cntrl < 0)
                {
                    cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                    return -1;                     // error occured, message will be given by exited funct
                }
                break;
            }
            cntrl = handleConnectors(&instructs, list, SEQ);
            if (cntrl < 0)
            {
                /* DO NOT CLEANUPINSTRUCT HERE! if  handleconnectors fails now new instruct was initialized 
                   i.e every currently existing malloced instruct is part of list, which will be freed as a
                   whole by main*/
                // error message thrown by exited funtion
                return -1;  // caller responsible for cleanup
            }
            idx = 0;  // a connector is the end of a token
            pos++;    // move position
            break;

        case '"':  // double quote
            // double quote loses its meaning in single quotes
            if (mode == SINGLE_QUOTES)
            {
                cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
                if (cntrl < 0)
                {
                    cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                    return -1;                     // error occured, message will be given by exited funct
                }
                break;
            }
            switchModes(&mode, text[pos]);
            pos++;  // this skips the quotes so they dont land in the token
            break;

        case 39:  // single quote
            // single quote loses its meaning in double quotes
            if (mode == DOUBLE_QUOTES) 
            {
                cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
                if (cntrl < 0)
                {
                    cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                    return -1;                     // error occured, message will be given by exited funct
                }
                break;
            }
            switchModes(&mode, text[pos]);
            pos++;  // this skips the quotes so they dont land in the token
            break;

        case '>':
            if (mode != NORMAL)
            {
                cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
                if (cntrl < 0)
                {
                    cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                    return -1;                     // error occured, message will be given by exited funct
                }
                break;
            }
            if (switchDirect(&dir, token, idx) == 0)
            {
                token[idx] = text[pos];
                pos++;
                idx++;
            }
            else
            {
                cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                return -1;                     // syntax error, handled by switchDirect
            }
            break;

        case '<':
            if (mode != NORMAL)
            {
                cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
                if (cntrl < 0)
                {
                    cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                    return -1;                     // error occured, message will be given by exited funct
                }
                break;
            }
            if (idx > 0)  // < can only stand alone
            {
                error = SYNTAX_ERROR;
                printError(error, &text[pos]);
                return -1;
            }
            dir = REDIR_IN;
            token[idx] = text[pos];  // to avoid segmentation faults, this will not end up in args
            pos++;
            idx++;
            break;

        default:
            cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
            if (cntrl < 0)
            {
                cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
                return -1;                     // error occured, message will be given by exited funct
            }
            
            break;
        }
    }

    // final token when text[pos] reaches \0
    cntrl = appendToToken(&instructs, token, text, &idx, &pos, dir);
    if (cntrl < 0)
    {
        cleanupInstructs(&instructs);  // nessecary clean up as this is instruct is not part of list yet
        return -1;                     // error occured, message will be given by exited funct
    }
    cntrl = handleConnectors(&instructs, list, END);
    if (cntrl < 0)
    {
        /* DO NOT CLEANUPINSTRUCT HERE! if  handleconnectors fails now new instruct was initialized 
                       i.e every currently existing malloced instruct is part of list, which will be freed as a
                     whole by main*/
        return -1;  // error occured, message will be given by exited funct
    }

    // if nothing was added to instructs we need to clean up locally
    if (list->size == 1)
    {
        cleanupInstructs(&instructs);
        return 0;
    }

    return 0;
}

/// @brief initializes a struct of type instructions
/// @param cmd
/// @return returns 0 if successful, -1 if not
int initInstructs(Instructions *cmd_)
{
    // initializes all struct variables
    for (int i = 0; i < 10; i++)
    {
        cmd_->redir[i].direction = NO_REDIR;
        cmd_->redir[i].stream = -1;
        cmd_->redir[i].target = NULL;
    }
    cmd_->capac = 30;
    cmd_->parseamt = 0;
    cmd_->rdrctns = 0;
    cmd_->args = NULL;
    cmd_->args = calloc(cmd_->capac, sizeof(char *));  // base length, gets doubled if nessecary
    if (cmd_->args == NULL)
    {
        perror("malloc");
        return -1;
    }

    return 0;
}

int initInstructList(InstructList *list)
{
    list->connects = malloc(sizeof(CONNECTOR));
    if (list->connects == NULL)
    {
        perror("malloc");
        return -1;
    }
    list->instructs = malloc(sizeof(Instructions));
    if (list->instructs == NULL)
    {
        perror("malloc");
        return -1;
    }
    list->size = 1;  // Sentinel slot
    return 0;
}

/// @brief function cleansup an entire instructlist struct
/// @param list
void cleanupInstructList(InstructList *list)
{
    // this frees every malloced argument token, and every conntector in the connects array
    for (int i = 0; i < list->size - 1; i++)
    {
        for (int j = 0; j < list->instructs[i].parseamt; j++)
        {
            free(list->instructs[i].args[j]);
        }
        free(list->instructs[i].args);  // frees the ralloc block of the given args array
        // this free all redirection targets of every instruction
        for (int j = 0; j < list->instructs[i].rdrctns; j++)
        {
            free(list->instructs[i].redir[j].target);
        }
    }

    free(list->instructs);  // frees the instructs array
    free(list->connects);   // frees the connects array
}

/// @brief frees allocated memory of a Instructions struct
/// @param cmd_
void cleanupInstructs(Instructions *cmd_)
{
    for (int i = 0; i < cmd_->parseamt; i++)
    {
        free(cmd_->args[i]);
    }
    free(cmd_->args);

    for (int i = 0; i < cmd_->rdrctns; i++)
    {
        free(cmd_->redir[i].target);
    }
}
