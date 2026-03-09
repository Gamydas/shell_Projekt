#include <stdio.h>
#include <termios.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "shell.h"
#include "str.h"
#include "escapesequenzen.h"
#include "tools.h"
#include "tab.h"

void handleArrows(shell *sh)
{
    char c = '\0';
    read(0, &c, 1);

    if (c != '[') // all arrow keys begin with \033[
    {
        return;
    }
    else
    {
        read(0, &c, 1);

        switch (c)
        {
        case 'A': // arrow up
            if (sh->histpos == 0)
            {
                printf("\a");
                fflush(stdout);
                return;
            } /* checks if oldest command has been reached*/
            if (sh->histpos > 0)
            {
                strcopy(sh->cmd, sh->hist[sh->histpos]);                // saves current input in history at histpos
                sh->histpos--;                                          // goes back into hist by 1
                strcopy(sh->hist[sh->histpos], sh->cmd);                // copies previous instruction into the command
                sh->cursoridx = strLen(sh->cmd) + strLen(sh->wdir) + 2; // repositioning cursoridx
            }
            break;

        case 'B': // arrow down
            if (sh->histpos == 50 || sh->hist[sh->histpos][0] == 0)
            {
                printf("\a");
                fflush(stdout);
                return;
            } /* checks if newest command has been reached*/
            strcopy(sh->cmd, sh->hist[sh->histpos]); // saves current input in history at histpos
            sh->histpos++;
            strcopy(sh->hist[sh->histpos], sh->cmd);                // copies previous instruction into the command
            sh->cursoridx = strLen(sh->cmd) + strLen(sh->wdir) + 2; // repositioning cursoridx
            break;
        case 'C':
            // checks if the curosr has met the rightmost edge of the command then increments cursor if not
            if (sh->cursoridx < strLen(sh->cmd) + strLen(sh->wdir) + 2)
            {
                sh->cursoridx++;
            }
            else
            {
                printf("\a");
                fflush(stdout);
                return;
            }
            break;
        case 'D':
            // checks if the curosr has met the leftmost edge of the command then decrements cursor if not
            if (sh->cursoridx > strLen(sh->wdir) + 2)
            {
                sh->cursoridx--;
            }
            else
            {
                printf("\a");
                fflush(stdout);
                return;
            }
            break;
        default: // other escape sequences starting with \033[ are ignored for now
            break;
        }
    }
}

/// @brief function to reposition cursor on the screen
/// @param sh
void reposCurs(shell *sh)
{
    // adjust cursor if <- or -> were pressed
    printf("\r");
    fflush(stdout);
    for (int i = 0; i < sh->cursoridx; i++)
    {
        printf("\033[C");
        fflush(stdout);
    }
}

/// @brief function lets the user input a string which will be interpreted as a command
/// @param Prompt giving prompt by calling function, i.e. cwd
/// @param cmd string which will be interpreted as a command
void getInput(shell *sh)
{
    tabComp tab;
    initTab(&tab);
    printf("%s: ", sh->wdir); // display prompt
    fflush(stdout);
    sh->cmd[0] = '\0'; // to check if nothing was input
    tcsetattr(0, TCSANOW, &sh->raw);
    char c = '\0';
    int len = 0;                    // variable checks how long the currently typed cmd is
    int anc = strLen(sh->wdir) + 2; // varible to memorize the beggining of the user editable part of the cmd line
    sh->cursoridx = anc;            // positions cursor at the first user eligable position
    while (1)
    {

        read(0, &c, 1);
        switch (c)
        {
        case '\t': // Tabulator to autocomplete the directory if possible
            if(tab.tabs == 0)
            {
                initTab(&tab);
            }
            tab.tabs++;
            tabComplete(&tab, sh->cmd, sh->wdir);
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes screen
            fflush(stdout);
            len = strLen(sh->cmd);     // readjusts str len
            sh->cursoridx = len + anc; // readjusts cursor
            break;

        case '\n': // user pressed enter to send their instruction
            if (sh->cmd[0] == '\0')
            {
                continue;
            }
            strcopy(sh->cmd, sh->hist[sh->histpos]);
            sh->histpos++;
            tab.tabs = 0; // resets Tab counter
            printf("\r\n");
            fflush(stdout);
            tcsetattr(0, TCSANOW, &sh->canon); // exits raw mode

            /*
            if ((addItem(&sh->history, sh->cmd)) == 0) // checks if adding item was successful
            {
                sh->latest = sh->history.size - 1;     // resets latest
            }
            */
            return;
        case 127:
            tab.tabs = 0; // resets Tab counter
            if (sh->cursoridx > anc)
            {
                len--;
                sh->cursoridx--;
                delInStr(sh->cmd, sh->cursoridx - anc);
                printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes the screen
                reposCurs(sh);
                fflush(stdout);
            }
            else
            {
                printf("\a");
                fflush(stdout);
            }

            break;

        case '\033': // checks for all inputs starting with \033 mainly for arrow keys, ignoring every other instruction that begins like this, might add more later
            handleArrows(sh);
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes the screen
            fflush(stdout);

            reposCurs(sh);
            len = strLen(sh->cmd);
            tab.tabs = 0; // resets Tab counter

            break;
        default:
            if (len == 200)
            {
                printf("\a");
                fflush(stdout);
                break;
            }
            // cursor is in the string not at the end
            if (sh->cursoridx - anc != len)
            {
                insertInStr(sh->cmd, c, sh->cursoridx - anc, (int)sizeof(sh->cmd));
                sh->cursoridx++;
                len++;
                sh->cmd[len] = '\0';
            }
            else
            { // cursor is at the end of the string
                sh->cmd[sh->cursoridx - anc] = c;
                sh->cursoridx++;
                len++;
                sh->cmd[len] = '\0';
            }
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshing screen
            reposCurs(sh);
            fflush(stdout);
            tab.tabs = 0; // resets Tab counter
            break;
        }
    }
}

/// @brief checks for the pipe instruction via "|" and handles it via fork/exec
/// @param sh
/// @return returns 0 if a pipeline happend, and -1 if not
int handlePipes(shell *sh)
{
    int i = 0;
    int pipecalls = 0;
    while (sh->instruc[i] != NULL)
    {
        if (strcomp(sh->instruc[i], "|") == 0 && i > 0)
        {
            pipecalls++;
        }
        i++;
    }

    if (pipecalls > 0)
    {

        // this block creates the nessecary args arrays for pipelining multiple processes later
        char **args[pipecalls + 1]; // not a pretty datastructe, i might look for ways to change this later
        for (int i = 0; i <= pipecalls; i++)
        {
            args[i] = malloc(50 * sizeof(char *));
            if (args[i] == NULL)
            {
                // frees all previously allocated memory
                for (int a = 0; a < i; a++)
                {
                    free(args[a]);
                }

                fprintf(stderr, "malloc failed\n");
                return -1;
            }
        }

        // this block fills the args arrays
        i = 0;         // counter to iterate over every string in instruc
        int count = 0; // another counter to iterate over every array string array in args
        int len = 0;   // anchor value to remember the length

        // increments i until it finds the pipecalls
        for (int p = 0; p <= pipecalls; p++)
        {
            while (sh->instruc[i] != NULL && strcomp(sh->instruc[i], "|"))
            {
                i++;
                len++;
            }

            /*
            The idea here is that on position [count][a] the pointer on position instruc[a + i - len] will be saved.
            i-len is always the beginning of a "word" (segement between or before/after pipecalls)
            */
            for (int a = 0; a < len; a++)
            {
                args[count][a] = sh->instruc[a + i - len];
            }
            i++;                     // incrementing i one further to move over the current "|"
            args[count][len] = NULL; // terminating array with NULL to pass it to exec later
            len = 0;                 // resetting len for next iteration
            count++;                 // incrementing count to save the next "word" (segment between or before/after piepcalls)
        }

        // this block creates all neccessary pipes
        int pipes[pipecalls][2];
        for (int a = 0; a < pipecalls; a++)
        {
            int p = pipe(pipes[a]);
            if (p < 0) // piping failed
            {
                // closes all opend pipes before returning
                for (int z = 0; z < a; z++)
                {
                    close(pipes[z][0]);
                    close(pipes[z][1]);
                }

                // frees all allocated memory before returning

                for (int i = 0; i <= pipecalls; i++)
                {
                    free(args[i]);
                }

                fprintf(stderr, "pipe failed\n");
                return -1;
            }
        }

        // this block creates all nessecarry children to execute the given command
        for (int p = 0; p <= pipecalls; p++)
        {
            int rc = fork();
            if (rc < 0) // forking failed
            {
                // closes all pipes before returning
                for (int i = 0; i < pipecalls; i++)
                {
                    close(pipes[i][0]);
                    close(pipes[i][1]);
                }

                // frees all allocated memory before returning
                for (int i = 0; i <= pipecalls; i++)
                {
                    free(args[i]);
                }

                fprintf(stderr, "fork failed\n");
                return -1;
            }
            if (rc == 0)
            {
                // this block connects all pipes according to the position in the command that the child takes
                if (p == 0) // first child, only writes
                {
                    dup2(pipes[0][1], STDOUT_FILENO);
                }
                else if (p == pipecalls) // last child, only reads
                {
                    dup2(pipes[pipecalls - 1][0], STDIN_FILENO);
                }
                else
                {
                    /* the rest of the created children will always be on the p-1 read end and p write end i
                    i.e child 1 needs to read from pipe 0 and write to pipe 1 and child 2 needs to read from
                    pipe 1 and write to pipe 2 and so forth*/
                    dup2(pipes[p - 1][0], STDIN_FILENO);
                    dup2(pipes[p][1], STDOUT_FILENO);
                }

                // closes all created pipes in the child
                for (int i = 0; i < pipecalls; i++)
                {
                    close(pipes[i][0]);
                    close(pipes[i][1]);
                }
                // executes command and prints an error if it does not exist, then exits(1)
                execvp(args[p][0], args[p]);
                fprintf(stderr, "%s: not a command\n", args[p][0]);
                exit(1);
            }
        }

        // closes all created pipes in the parent
        for (int i = 0; i < pipecalls; i++)
        {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        while (wait(0) > 0)
            ; // waits for all children to finish

        // frees all allocated memory
        for (int i = 0; i <= pipecalls; i++)
        {
            free(args[i]);
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

/// @brief redirects stream to a chosen file
/// @param text
/// @param filename
/// @return returns 0 if redirection was succesful, -1 otherwise
int redirect(shell *sh)
{
    int i = 0;

    // flags used to mark the position(and check its existence) of the operator in the string
    // flag[0] for >
    // flag[1] for >>
    // flag[2] for 2>
    // flag[3] for 2>>
    // flag[4] for <
    int flag[] = {-1, -1, -1, -1, -1};

    // variables used to store file descriptors
    int dataout = STDOUT_FILENO;
    int dataerr = STDERR_FILENO;
    int datain = STDIN_FILENO;
    while (sh->instruc[i] != NULL)
    {
        if (!strcomp(sh->instruc[i], ">"))
        {
            flag[0] = i;
        }
        if (!strcomp(sh->instruc[i], ">>"))
        {
            flag[1] = i;
        }
        if (!strcomp(sh->instruc[i], "2>"))
        {
            flag[2] = i;
        }
        if (!strcomp(sh->instruc[i], "2>>"))
        {
            flag[3] = i;
        }
        if (!strcomp(sh->instruc[i], "<"))
        {
            flag[4] = i;
        }
        i++;
    } /* looks for the redirection operators and if found saves
         their position in the appropriate flag*/

    // no redirection operator has been called
    if (flag[0] == -1 && flag[1] == -1 && flag[2] == -1 && flag[3] == -1 && flag[4] == -1)
    {
        return -1;
    }

    // fetching filedescriptors in appropriate modes
    if (flag[0] != -1)
    {
        dataout = open(sh->instruc[flag[0] + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if (flag[1] != -1)
    {
        if (dataout == STDOUT_FILENO)
        {
            dataout = open(sh->instruc[flag[1] + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
    }
    if (flag[2] != -1)
    {
        dataerr = open(sh->instruc[flag[2] + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if (flag[3] != -1)
    {
        if (dataerr == STDERR_FILENO)
        {
            dataerr = open(sh->instruc[flag[3] + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
    }
    if (flag[4] != -1)
    {
        datain = open(sh->instruc[flag[4] + 1], O_RDONLY);
    }

    int rc = fork();
    if (rc < 0)
    {
        // closing all opend streams to avoid leaks
        close(dataout);
        close(dataerr);
        close(datain);
        fprintf(stderr, "fork failed\n");
        return -1;
    }

    // child
    if (rc == 0)
    {

        // setting up streams
        if (dataout != STDOUT_FILENO)
        {
            dup2(dataout, STDOUT_FILENO);
            close(dataout);
        }
        if (dataerr != STDERR_FILENO)
        {
            dup2(dataerr, STDERR_FILENO);
            close(dataerr);
        }
        if (datain != STDIN_FILENO)
        {
            dup2(datain, STDIN_FILENO);
            close(datain);
        }

        // command is first in the string
        int min = findMinXn1(flag, 5);
        if (min != 0)
        {
            // this block cuts the instruc array off at the point of redirection so command does not read invalid arguments
            sh->instruc[min] = NULL;
            execvp(sh->instruc[0], sh->instruc);
        }
        else if (min == 0) // redirection instructions come first
        {

            int max = findMax(flag, 5);
            execvp(sh->instruc[max + 2], &sh->instruc[max + 2]); // +2 to offset last redirect + targeted file
        }
        fprintf(stderr, "%s: not a command\n", sh->instruc[0]); // if execvp cannot find the given command
        exit(1);
    }
    else
    {
        // this block checks if any new filedescriptors were opend and closes them if so
        if (dataout != STDOUT_FILENO)
            close(dataout);
        if (dataerr != STDERR_FILENO)
            close(dataout);
        if (datain != STDIN_FILENO)
            close(dataout);
        wait(0);
    }

    return 0;
}
