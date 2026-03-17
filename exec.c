#include "exec.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.h"
#include "pipelining.h"
#include "str.h"

/// @brief this function handles the execution logic of the shell, it gets handed an InstructList
///        and the functiontable + its size. It checks for Connector Types and handles them
///        in the intended way, this now enables for better improved expandability and
///        maintainability, redirections in pipes are possible, different connectors can be called
///        at once i.e ls | grep .c ; ls main or the like
/// @param list
/// @param builtins
/// @param binamt
int executeInstructs(InstructList *list, Builtin *builtins, int binamt)
{
    int pipecalls = 0;
    int ID = PARENT_PROCCESS;
    int cntrl = 0;
    int begin = -1;
    for (int i = 0; i < list->size - 1; i++)
    {
        // groups together every consecutive segment of the pipe
        while (list->connects[i] == PIPE)
        {
            if (begin == -1) begin = i;  // checks if begin has been set and sets it if not
            pipecalls++;
            i++;
        }
        if (pipecalls > 0)
        {
            ID = setUpPipes(&list->instructs[begin], pipecalls);
            if (ID < 0)
            {
                return -1;  // caller handles cleanup
            }
            if (ID != PARENT_PROCCESS)  // to avoid executing commands twice
            {
                // executes commands in the children at their approricate position i.E instructs[begin + ID]
                cntrl = executeCommand(&list->instructs[begin + ID], builtins, binamt, ID);
                // execCommand terminates the children after execution, no need to do that here
                if (cntrl < 0)
                {
                    return -1;  // caller handles cleanup
                }
                exit(0);
            }
        }
        // if last segment was part of a pipe do nothing ie. ls | grep .c ; ls - the ; doesnt want to execute the grep, neither das end if that was the last connector
        if ((list->connects[i] == SEQ || list->connects[i] == END) && pipecalls > 0)
        {
            pipecalls = 0;  // resets pipeline
            begin = -1;     // resets begin
            continue;
        }
        else
        {
            cntrl = executeCommand(&list->instructs[i], builtins, binamt, ID);
            if (cntrl < 0)
            {
                return -1;  // caller handles cleanup
            }
            if (cntrl == 1)  // indicates exit has been called
            {
                return 1;
            }
        }
        // add more seperators here
        pipecalls = 0;  // resets pipeline
        begin = -1;     // resets begin
    }
    return 0;
}

/// @brief executes a shell command, i.e a builtin or executable, gets handed an ID
///        to identify if the caller is child or parent, terminates children after
///        command execution, also handles errors
/// @param instructs
/// @param builtins
/// @param binamt
/// @param ID
/// @return 0 if success, -1 if error, 1 if exit is called
int executeCommand(Instructions *instructs, Builtin *builtins, int binamt, int ID)
{
    // local copies of the standard datachannels to restore them after redirections
    int out_fd = dup(STDOUT_FILENO);
    int err_fd = dup(STDERR_FILENO);
    int in_fd = dup(STDIN_FILENO);

    int cntrl = 0;
    // sets up redirections for parents process, redirects for pipes are handles by setUpPipe
    if (ID == PARENT_PROCCESS)
    {
        for (int i = 0; i < instructs->rdrctns; i++)
        {
            cntrl = handleRedirections(&instructs->redir[i]);
            if (cntrl < 0)
            {
                return -1;
            }
        }
    }
    /* catches if an instruct has empty arguments, this can happen if e.g only a redirection
       was part of an instruction i.e > out.txt or < in.txt | grep .c | ; this is due to
       redirections and other operators not being tokenized*/
    if (instructs->args[0] == NULL)
    {
        // in casethis happens in a pipe or other child processes
        if (ID != PARENT_PROCCESS)
        {
            exit(0);
        }
        // restoring original FDs
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
        dup2(err_fd, STDERR_FILENO);
        close(out_fd);
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
        return 0;  // silent return to main in parent
    }
    // checks for built ins
    for (int i = 0; i < binamt; i++)
    {
        if (strcomp(builtins[i].name, instructs->args[0]) == 0)
        {
            // only for the parent, i.e the actual shell process
            if (strcomp(builtins[i].name, "exit") == 0 && ID == PARENT_PROCCESS)
            {
                return 1;  // indicates caller that exit has been called
            }
            builtins[i].bin(instructs->args[1]);
            // terminates process if called by child
            if (ID != PARENT_PROCCESS)
            {
                exit(0);
            }
            else
            {
                // restoring original fds after a builtin
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
                dup2(err_fd, STDERR_FILENO);
                close(out_fd);
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
                return 0;
            }
        }
    }
    // if this was called by a child e.g a pipe segment there is no need to create new children
    if (ID != PARENT_PROCCESS)
    {
        execvp(instructs->args[0], instructs->args);
        perror("execvp");
        exit(1);
    }
    else if (ID == PARENT_PROCCESS)  // parent called this function
    {
        int rc = fork();
        if (rc < 0)
        {
            perror("fork");
            return -1;
        }
        else if (rc == 0)
        {
            execvp(instructs->args[0], instructs->args);
            perror("execvp");
            exit(1);
        }
        else
        {
            wait(0);
        }
    }
    dup2(out_fd, STDOUT_FILENO);
    close(out_fd);
    dup2(err_fd, STDERR_FILENO);
    close(out_fd);
    dup2(in_fd, STDIN_FILENO);
    close(in_fd);
    return 0;
}