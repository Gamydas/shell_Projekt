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
#include "parser.h"

/// @brief function to handle escapesequences, mainly the arrow keys for history navigation
/// @param sh
/// @param cmd_
/// @return 0 if success, -1 if an error occured
int handleArrows(shell *sh, command *cmd_)
{
    char c = '\0';
    int rd = read(0, &c, 1);
    if (rd < 0) // checks failed read
    {
        perror("read");
        return -1;
    }
    else if (rd == 0) // checks end of file
    {
        return -1;
    }

    if (c != '[') // all arrow keys begin with \033[
    {
        return 0; // there are escape sequences that dont follow with [ so no -1
    }
    else
    {
        int rd = read(0, &c, 1);
        if (rd < 0) // checks failed read
        {
            perror("read");
            return -1;
        }
        else if (rd == 0) // checks end of file
        {
            return -1;
        }

        switch (c)
        {
        case 'A': // arrow up
            if (sh->histpos == 0)
            {
                printf("\a");
                fflush(stdout);
                break;
            } /* checks if oldest command has been reached*/
            if (sh->histpos > 0)
            {
                strcopy(cmd_->cmd, sh->hist[sh->histpos]);                  // saves current input in history at histpos
                sh->histpos--;                                              // goes back into hist by 1
                strcopy(sh->hist[sh->histpos], cmd_->cmd);                  // copies previous instruction into the command
                cmd_->cursoridx = strLen(cmd_->cmd) + strLen(sh->wdir) + 2; // repositioning cursoridx
            }
            break;

        case 'B': // arrow down
            if (sh->histpos == 50 || sh->hist[sh->histpos][0] == 0)
            {
                printf("\a");
                fflush(stdout);
                break;
            } /* checks if newest command has been reached*/
            strcopy(cmd_->cmd, sh->hist[sh->histpos]); // saves current input in history at histpos
            sh->histpos++;
            strcopy(sh->hist[sh->histpos], cmd_->cmd);                  // copies previous instruction into the command
            cmd_->cursoridx = strLen(cmd_->cmd) + strLen(sh->wdir) + 2; // repositioning cursoridx
            break;
        case 'C':
            // checks if the curosr has met the rightmost edge of the command then increments cursor if not
            if (cmd_->cursoridx < strLen(cmd_->cmd) + strLen(sh->wdir) + 2)
            {
                cmd_->cursoridx++;
            }
            else
            {
                printf("\a");
                fflush(stdout);
                break;
            }
            break;
        case 'D':
            // checks if the curosr has met the leftmost edge of the command then decrements cursor if not
            if (cmd_->cursoridx > strLen(sh->wdir) + 2)
            {
                cmd_->cursoridx--;
            }
            else
            {
                printf("\a");
                fflush(stdout);
                break;
            }
            break;
        default: // other escape sequences starting with \033[ are ignored for now
            break;
        }
    }
    return 0;
}

/// @brief function to reposition cursor on the screen
/// @param cursoridx value that indicates where the curoser is currently positioned
void reposCurs(int cursoridx)
{
    // adjust cursor if <- or -> were pressed
    printf("\r");
    fflush(stdout);
    for (int i = 0; i < cursoridx; i++)
    {
        printf("\033[C");
        fflush(stdout);
    }
}

/// @brief function lets the user input a string which will be interpreted as a command
/// @param Prompt giving prompt by calling function, i.e. cwd
/// @param cmd string which will be interpreted as a command
/// @return returns length of read input, -1 if an error occured
int getInput(shell *sh, command *cmd_)
{
    tabComp tab;
    int len = 0;                    // variable checks how long the currently typed cmd is
    int anc = strLen(sh->wdir) + 2; // varible to memorize the beggining of the user editable part of the cmd line
    char c = '\0';
    cmd_->cursoridx = anc; // positions cursor at the first user eligable position

    initTab(&tab);
    tcsetattr(0, TCSANOW, &sh->raw); // enters raw mode

    printf("%s: ", sh->wdir); // display prompt
    fflush(stdout);

    while (1)
    {

        int rd = read(0, &c, 1);
        if (rd < 0) // checks failed read
        {
            perror("read");
            tcsetattr(0, TCSANOW, &sh->canon); // return to canon mode
            return -1;
        }
        else if (rd == 0) // checks end of file
        {
            tcsetattr(0, TCSANOW, &sh->canon); // return to canon mode
            return -1;
        }
        switch (c)
        {
        case '\t': // Tabulator to autocomplete the directory if possible
            if (tab.tabs == 0)
            {
                initTab(&tab);
            }
            tab.tabs++;
            tabComplete(&tab, sh->builtins, cmd_->cmd, sh->wdir);

            // refreshes screen
            printf("\r\033[K%s: %s", sh->wdir, cmd_->cmd);
            fflush(stdout);

            len = strLen(cmd_->cmd);     // readjusts str len
            cmd_->cursoridx = len + anc; // readjusts cursor
            break;

        case '\n': // user pressed enter to send their instruction
            if (cmd_->cmd[0] == '\0')
            {
                continue;
            }
            // checks for history buffer overflows
            if (sh->histpos < 50)
            {
                strcopy(cmd_->cmd, sh->hist[sh->histpos]);
                sh->histpos++;
            }

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
            return strLen(cmd_->cmd);
        case 127:
            tab.tabs = 0; // resets Tab counter
            if (cmd_->cursoridx > anc)
            {
                len--;
                cmd_->cursoridx--;
                delInStr(cmd_->cmd, cmd_->cursoridx - anc); // removes item in string

                // refreshes the screen
                printf("\r\033[K%s: %s", sh->wdir, cmd_->cmd);
                fflush(stdout);
                reposCurs(cmd_->cursoridx);
            }
            else
            {
                printf("\a");
                fflush(stdout);
            }

            break;

        case '\033': // checks for all inputs starting with \033 mainly for arrow keys, ignoring every other instruction that begins like this, might add more later
            if(handleArrows(sh, cmd_) < 0)
            {
                tcsetattr(0, TCSANOW, &sh->canon); // return to canon mode
                return -1;
            }
            // refreshes the screen
            printf("\r\033[K%s: %s", sh->wdir, cmd_->cmd);
            fflush(stdout);
            reposCurs(cmd_->cursoridx);

            len = strLen(cmd_->cmd);
            tab.tabs = 0; // resets Tab counter

            break;
        default:
            // checks if size of cmd string needs to be increased
            if (len == cmd_->capac)
            {
                cmd_->capac *= 2; // doubles capacity
                char *temp = realloc(cmd_->cmd, cmd_->capac);
                if (temp == NULL)
                {
                    perror("malloc");
                    tcsetattr(0, TCSANOW, &sh->canon); // return to canon mode
                    return -1;
                }

                cmd_->cmd = temp;
            }
            // cursor is in the string not at the end
            if (cmd_->cursoridx - anc != len)
            {
                insertInStr(cmd_->cmd, c, cmd_->cursoridx - anc, strLen(cmd_->cmd));
                cmd_->cursoridx++;
                len++;
                cmd_->cmd[len] = '\0';
            }
            else
            { // cursor is at the end of the string
                cmd_->cmd[cmd_->cursoridx - anc] = c;
                cmd_->cursoridx++;
                len++;
                cmd_->cmd[len] = '\0';
            }

            // refreshing screen
            printf("\r\033[K%s: %s", sh->wdir, cmd_->cmd);
            fflush(stdout);
            reposCurs(cmd_->cursoridx);

            tab.tabs = 0; // resets Tab counter
            break;
        }
    }
}

/*
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
*/
            /*
            The idea here is that on position [count][a] the pointer on position instruc[a + i - len] will be saved.
            i-len is always the beginning of a "word" (segement between or before/after pipecalls)
            */
/*
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

/*
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
*/