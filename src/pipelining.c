#include "pipelining.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "parser.h"
#include "redirect.h"
#include "str.h"

/// @brief checks for the pipe instruction via "|" and handles it via fork/exec
/// @param sh
/// @return returns 0 if a pipeline happend, and -1 if not
int setup_pipes(Instructions *instruct, int size)
{
    int pipes[size][2];  // these will be the I/O pipes for the pipeline

    // files opens the I/O Pipes
    for (int i = 0; i < size; i++)
    {
        int p = pipe(pipes[i]);
        // checks for errors during piping and cleans up if one occured
        if (p < 0)
        {
            perror("pipe");
            close_pipes(pipes, i);
            return -1;
        }
    }

    for (int i = 0; i <= size; i++)
    {
        int rc = fork();
        if (rc < 0)  // forking failed
        {
            perror("fork");
            close_pipes(pipes, size);
            return -1;
        }
        else if (rc == 0)
        {
            // handles all redirection calls in the pipe segments and handles erros if they happen
            for (int j = 0; j < instruct[i].rdrctns; j++)
            {
                int cntrl = handle_redirections(&instruct[i].redir[j]); 
                if (cntrl < 0)
                {
                    close_pipes(pipes, size);
                    exit(1);
                }
            }

            if (i == 0)  // first pipe
            {
                int a = dup2(pipes[0][1], STDOUT_FILENO);
                if (a < 0)
                {
                    perror("dup2");
                    close_pipes(pipes, size);
                    exit(1);
                }
            }
            else if (i == size)  // last pipe
            {
                int a = dup2(pipes[size - 1][0], STDIN_FILENO);
                if (a < 0)
                {
                    perror("dup2");
                    close_pipes(pipes, size); 
                    exit(1);
                }
            }
            else
            {
                /* every child beteen 0 and size is a double sided pipe
                   and is always on the i-1 read and and the i write end*/
                int a = dup2(pipes[i - 1][0], STDIN_FILENO);
                if (a < 0)
                {
                    perror("dup2");
                    close_pipes(pipes, size);
                    exit(1);
                }
                a = dup2(pipes[i][1], STDOUT_FILENO);
                if (a < 0)
                {
                    perror("dup2");
                    close_pipes(pipes, size);
                    exit(1);
                }
            }
            close_pipes(pipes, size);
            return i;  // returns position in the pipe
        }
    }

    // closing all pipes in parent to avoid leaks
    close_pipes(pipes, size);
    // parent waits for all children to terminate
    while (wait(0) > 0);
    return PARENT_PROCCESS;
}

/// @brief function closes every opend pipe it gets pased
/// @param pipes a pipe[size][2] array of pipes to close
/// @param size amount of pipe[2] arrays
void close_pipes(int pipes[][2], int size)
{
    // closes all pipes before returning
    for (int z = 0; z < size; z++)
    {
        close(pipes[z][0]);
        close(pipes[z][1]);
    }
}