#include <stdio.h>
#include <termios.h>
#include <dirent.h>
#include "shell.h"
#include "str.h"
#include "escapesequenzen.h"
#include "list.h"
#include "shellcmd.h"

/// @brief support function which gets passed a shell struc and looks through present directories and looks for either complete or partial matches,
///        which will then be saved to the twins array(later the entire array will be printed out on a double press of TAB)
/// @param sh
void tabComplete(shell *sh)
{
    DIR *temp = opendir(sh->wdir);
    struct dirent *temp2;
    char tempdir[256];
    int count = 0;

    // moving to the first uncompleted part of the cmd string
    int i = 0;
    while (sh->cmd[i] != '\0')
    {
        while (sh->cmd[i] == 32 || sh->cmd[i] == '/')
        {
            i++;
            if (sh->cmd[i] != 32 && sh->cmd[i] != '\0' && sh->cmd[i] != '/')
            {
                count = i;
            }
        }
        i++;
    }

    int c2 = count;      // quickfix for memorising first letter of new cmd segment
    char twins[50][256]; // array used to save identical option incase there are multiple similarily named files
    int twidx = 0;       // index of twins array to properly save
    while ((temp2 = readdir(temp)) != NULL)
    {
        strcopy(temp2->d_name, tempdir);
        while (sh->cmd[count] == tempdir[count - c2])
        {
            count++;
            if (sh->cmd[count] == '\0')
            {
                strcopy(tempdir, twins[twidx]);
                twidx++;
            }
        }
        count = c2;
    }
    if (twidx == 0)
    {
        printf("\a"); // plays error sound
        fflush(stdout);
    }
    if (twidx == 1) // only checks if there even is an entry in twins
    {
        strcopy(twins[0], &sh->cmd[c2]);
    }
    if (twidx > 1) // more than one compatible option
    {
        char prefix[256]; // 255 is the max length of characters in a filename under Linux
        findPrefix(twins, prefix, twidx, 256);
        strcopy(prefix, &sh->cmd[c2]);
    }

    closedir(temp);
}

void handleArrows(shell *sh)
{

    list *hist = &sh->history;
    char c;
    read(0, &c, 1);
    // printf("read 1: %c\n", c);
    // fflush(stdout);
    if (c != '[')
    {
        return;
    }
    else
    {
        read(0, &c, 1);
        // printf("read 2: %c\n", c);
        // fflush(stdout);
        switch (c)
        {
        case 'A':
            strcopy(sh->cmd, hist->list[hist->size]);
            if (fetchItem(hist, &sh->latest) == NULL)
            {
                return;
            }
            strcopy(fetchItem(hist, &sh->latest), sh->cmd);

            if (sh->latest > 0)
            {
                sh->latest--;
            }
            sh->cursoridx = strLen(sh->cmd) + strLen(sh->wdir) + 2;
            break;
        case 'B':
            if (sh->latest < hist->size - 1)
            {
                sh->latest++;
            }

            if (fetchItem(hist, &sh->latest) == NULL)
            {
                return;
            }
            strcopy(fetchItem(hist, &sh->latest), sh->cmd);
            sh->cursoridx = strLen(sh->cmd) + strLen(sh->wdir) + 2;
            break;
        case 'C':
            if (sh->cursoridx < strLen(sh->cmd) + strLen(sh->wdir) + 2)
            {
                sh->cursoridx++;
            }

            break;
        case 'D':
            if (sh->cursoridx > strLen(sh->wdir) + 2)
            {
                sh->cursoridx--;
            }
            break;
        default:
            break;
        }
    }
}

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
    printf("%s: ", sh->wdir); // Prompt auf Bildschirm ausgeben
    fflush(stdout);
    sh->cmd[0] = '\0'; // Zur Kontrolle falls Benutzer nichts eingibt
    tcsetattr(0, TCSANOW, &sh->raw);
    char c;
    int len = 0;
    int anc = strLen(sh->wdir) + 2; // varible to memorize the beggining of the user editable part of the cmd line
    sh->cursoridx = anc;
    while (1)
    {

        read(0, &c, 1);
        switch (c)
        {
        case '\t': // Tabulator to autocomplete the directory if possible
            tabComplete(sh);
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes screen
            fflush(stdout);
            len = strLen(sh->cmd);
            break;

        case '\n': // user pressed enter to send their instruction
            if (sh->cmd[0] == '\0')
            {
                continue;
            }

            printf("\r\n");
            fflush(stdout);
            tcsetattr(0, TCSANOW, &sh->canon);

            if ((addItem(&sh->history, sh->cmd)) == 0) // checks if adding item was successful
            {
                sh->latest = sh->history.size - 1; // resets latest (-2 due to size beginning at 1 not 0)
            }

            return;
            break;
        case 127:
            if (sh->cursoridx > anc)
            {
                len--;
                sh->cursoridx--;
                delInStr(sh->cmd, sh->cursoridx - anc);
                printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes the screen
                reposCurs(sh);
                fflush(stdout);
            }
            break;

        case '\033': // checks for all inputs starting with \033 mainly for arrow keys, ignoring every other instruction that begins like this, might add more later
            handleArrows(sh);
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes the screen
            fflush(stdout);

            reposCurs(sh);
            len = strLen(sh->cmd);
            break;
        default:
            if (len == 200)
            {
                break;
            }
            sh->cmd[sh->cursoridx - anc] = c;
            sh->cursoridx++;
            len++;
            sh->cmd[len] = '\0';
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshing screen
            fflush(stdout);
            break;
        }
    }
}
