#ifndef PIPELINING_H
#define PIPELINING_H

typedef enum PIPESTATUS
{
    NO_PIPE,
    IN_PIPE,       // arg | 
    DOUBLE_PIPE,   // | arg |
    OUT_PIPE,      // | arg
} PIPESTATUS;

typedef struct pipeline
{
    PIPESTATUS status;
    char** args; // all arguments belonging to a pipe e.g ls -la | 
    int capac;   // capacity of array i.e first array dimension
    int pos;     // position in pipeline, needed to discern which pipe ends need to be connected 
} pipeline;

typedef struct parsedInput command;

int initPipe(pipeline *pipe_);
int handlePipes(pipeline *pipe_, int size);
int fillPipes(command *cmd_, int argarmt);
void closePipes(int pipes[][2], int size);


#endif