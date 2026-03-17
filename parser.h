#ifndef PARSER_H
#define PARSER_H
#include "redirect.h"

typedef enum
{
    SINGLE_QUOTES,
    DOUBLE_QUOTES,
    NORMAL,

} MODUS;


typedef enum
{
    NO_CONNECTOR, // value to use when initializing
    PIPE,         // | pipe 
    SEQ,          // ; sequential operator
    END           // no connector found
} CONNECTOR;

typedef struct parsedInput
{
    char** args;         // array in which parsed tokens will be saved
    int parseamt;       // current fill amount of parsed
    int capac;          // capacity of parsed array, i.e amount of currently allocated slots
    redirect redir[10]; // why would you ever need more than 10 in 1 cmd
    int rdrctns;        // amount of redirectons
} Instructions;

typedef struct 
{
   Instructions* instructs;  // array of instructions, I.e every token until a connector is found
   CONNECTOR* connects;       // array of connectors, connects[0] is the connector between instructs[0] and instructs[1]
   int size;                 // size of given instruc list 
} InstructList;


int addSegment(InstructList *list, Instructions *instructs, CONNECTOR connect);
int handleConnectors(Instructions *instructs, InstructList *list, CONNECTOR connect);
int handleSeperator(Instructions *instruct, char *token, char *text, int *idx, int *pos, REDIR *type);
int parseInput(InstructList *list, char *text);
void switchModes(MODUS *mode, char c);
int initInstructs(Instructions *instructs);
int initInstructList(InstructList *list);
void cleanupInstructs(Instructions *instructs);
void cleanupInstructList(InstructList *list);
#endif