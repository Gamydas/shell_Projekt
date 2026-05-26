#ifndef PARSER_H
#define PARSER_H
#include "redirect.h"

typedef enum
{
    SINGLE_QUOTES,
    DOUBLE_QUOTES,
    NORMAL,

} parseMode;

typedef enum
{
    NO_CONNECTOR,  // value to use when initializing
    PIPE,          // | pipe
    SEQ,           // ; sequential operator
    END            // no connector found
} CONNECTOR;

typedef struct parsedInput
{
    char **args;         // array in which parsed tokens will be saved
    int parseamt;        // current fill amount of parsed
    int capac;           // capacity of parsed array, i.e amount of currently allocated slots
    redirect redir[10];  // why would you ever need more than 10 in 1 cmd
    int rdrctns;         // amount of redirectons
} Instructions;

typedef struct
{
    Instructions *instructs;  // array of instructions, I.e every token until a connector is found
    CONNECTOR *connects;      // array of connectors, connects[0] is the connector between instructs[0] and instructs[1]
    int size;                 // size of given instruc list
} InstructList;

int add_segment(InstructList *list, Instructions *instructs, CONNECTOR connect);
int handle_connectors(Instructions *instructs, InstructList *list, CONNECTOR connect);
int handle_seperators(Instructions *instruct, char *token, char *text, int *idx, int *pos, REDIR *type);
int append_to_token(Instructions *instruct, char *token, char *text, int *idx, int *pos, REDIR type);
int parse_input(InstructList *list, char *text);
void switch_modes(parseMode *mode, char c);
int initialize_instructs(Instructions *instructs);
int initialize_instruct_list(InstructList *list);
void cleanup_instructs(Instructions *instructs);
void cleanup_instruct_list(InstructList *list);
#endif