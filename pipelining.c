#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "pipelining.h"
#include "parser.h"
#include "str.h"

/// @brief initializes one segment of a pipeline
/// @param pipe_
/// @return 0 for success, -1 for error
int initPipe(pipeline *pipe_)
{
    // base capacity, will be increaed via doubling if needed
    pipe_->capac = 0;
    pipe_->pos = -1; // needs to be manually set by caller
    pipe_->status = NO_PIPE;
    // caller is responsible for allocation of memory
    pipe_->args = NULL;
    return 0;
}

/// @brief creates pipe segments for a given command struct
/// @param cmd_
/// @param argamt passes amount of arguments for pipe segment
/// @return returns 0 if success and -1 if an error occured
int fillPipes(command *cmd_, int argamt)
{
    // local pointers to cmd_ filds for better readability
    int *parseamt = &cmd_->parseamt;
    int *pipecalls = &cmd_->pipecalls;
    *pipecalls += 1;

    // this block allocates new memory for a pipe element and initializes it
    pipeline *temp = realloc(cmd_->pipes, *pipecalls * sizeof(pipeline));
    if (temp == NULL)
    {
        perror("malloc");
        return -1;
    }
    cmd_->pipes = temp;
    initPipe(&cmd_->pipes[*pipecalls - 1 ]);

    // saves idx for the current pipe
    cmd_->pipes[*pipecalls - 1].pos = *pipecalls - 1; // pipecalls is an amt not an idx so -1
    int idx = cmd_->pipes[*pipecalls - 1].pos;

    // allocates memory for the argments of this pipe segment
    cmd_->pipes[idx].capac = argamt;                           // idea is right, execution wrong
    char **temp2 = realloc(cmd_->pipes[idx].args, (argamt + 1) * sizeof(char*)); // +1 for sentinel
    if (temp2 == NULL)
    {
        // dont free here, cleanupCMD does that already
        //free(cmd_->pipes);
        perror("malloc");
        return -1;
    }
    cmd_->pipes[idx].args = temp2;
    for (int i = 0; i < argamt; i++)
    {
        // argamt = *parseamt - segment -> segment = *parseamt - argamt i.e this is the starting point
        cmd_->pipes[idx].args[i] = cmd_->parsed[*parseamt - argamt + i]; // pointer copy to the desired arguments
    }
    cmd_->pipes[idx].args[argamt] = NULL; // places sentinel
    return 0;
}

/// @brief checks for the pipe instruction via "|" and handles it via fork/exec
/// @param sh
/// @return returns 0 if a pipeline happend, and -1 if not
int handlePipes(pipeline *pipe_, int size)
{
    int pipes[size][2]; // these will be the I/O pipes for the pipeline

    // files opens the I/O Pipes
    for (int i = 0; i < size; i++)
    {
        int p = pipe(pipes[i]);
        // checks for errors during piping and cleans up if one occured
        if (p < 0)
        {
            perror("pipe");
            closePipes(pipes,i);
            return -1;
        }
    }
    
    for (int i = 0; i <= size; i++)
    {
        int rc = fork();
        if (rc < 0) // forking failed
        {
            perror("fork");
            closePipes(pipes, size);
            return -1;
        }
        else if (rc == 0)
        {
            if (i == 0) // first pipe
            {
                int a = dup2(pipes[0][1], STDOUT_FILENO);
                if (a < 0)
                {
                    perror("dup2");
                    closePipes(pipes, size);
                    exit(1);
                }
            } else if (i == size) // last pipe
            {
                int a = dup2(pipes[size - 1][0], STDIN_FILENO);
                if (a < 0)
                {
                    perror("dup2");
                    closePipes(pipes, size);
                    exit(1);
                }
            } else
            {
                /* every child beteen 0 and size is a double sided pipe
                   and is always on the i-1 read and and the i write end*/
                int a = dup2(pipes[i - 1][0], STDIN_FILENO);
                if (a < 0)
                {

                    perror("dup2");
                    closePipes(pipes, size);
                    exit(1);
                }
                a = dup2(pipes[i][1], STDOUT_FILENO);
                if (a < 0)
                {
                    perror("dup2");
                    closePipes(pipes, size);
                    exit(1);
                }
            }
            
            // closes all pipes in children after dup2 to avoid leaks
            closePipes(pipes, size);
            execvp(pipe_[i].args[0], pipe_[i].args);
            fprintf(stderr, "%s: not a command\n", pipe_[i].args[0]);
            exit(1);
        }
    }

    // closing all pipes in parent to avoid leaks
    closePipes(pipes, size);
    // parent waits for all children to terminate
    while(wait(0) > 0);
    return 0;
}

/// @brief function closes every opend pipe it gets pased
/// @param pipes a pipe[size][2] array of pipes to close
/// @param size amount of pipe[2] arrays
void closePipes(int pipes[][2], int size)
{
    // closes all pipes before returning
    for (int z = 0; z < size; z++)
    {
        close(pipes[z][0]);
        close(pipes[z][1]);
    }
}