#include <stdio.h>
#include <termios.h>
#include <dirent.h>
#include "shell.h"
#include "str.h"
#include "escapesequenzen.h"
#include "circBuff.h"

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

    charCircBuff *hist = &sh->history;
    char c;
    read(0, &c, 1);
    //printf("read 1: %c\n", c);
    //fflush(stdout);
    if (c != '[')
    {
        return;
    }
    else
    {
        read(0, &c, 1);
        //printf("read 2: %c\n", c);
        //fflush(stdout);
        switch (c)
        {
        case 'A':
            tempadd2Buf(hist, sh->cmd);  // remember currently written contents of the terminal
            fetchLatest(hist, sh->cmd);  // fetches latest instruction
            break;
        case 'B':
            revFetchLatest(hist, sh->cmd);
            break;
        case 'C':
            printf("\033[C");
            fflush(stdout);
            break;
        case 'D':
            printf("\033[D");
            fflush(stdout);
            break;
        default:
            break;
        }
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
    int idx = 0;

    while (1)
    {

        read(0, &c, 1);
        switch (c)
        {
        case '\t': // Tabulator to autocomplete the directory if possible
            tabComplete(sh);
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes screen
            fflush(stdout);
            idx = strLen(sh->cmd);
            break;

        case '\n': // user pressed enter to send their instruction
            if (sh->cmd[0] == '\0')
            {
                continue;
            }
            printf("\r\n");
            fflush(stdout);
            tcsetattr(0, TCSANOW, &sh->canon);
            add2Buf(&sh->history,sh->cmd);
            resetLatest(&sh->history);
            return;
            break;
        case 127:
            if (idx > 0)
            {
                idx--;
                sh->cmd[idx] = '\0';
                printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes the screen
                fflush(stdout);
            }
            break;

        case '\033': // checks for all inputs starting with \033 mainly for arrow keys, ignoring every other instruction that begins like this, might add more later
            handleArrows(sh);
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshes the screen
            idx = strLen(sh->cmd);
            fflush(stdout);
            break;
        default:
            if (idx == 200)
            {
                break;
            }
            sh->cmd[idx] = c;
            idx++;
            sh->cmd[idx] = '\0';
            printf("\r\033[K%s: %s", sh->wdir, sh->cmd); // refreshing screen
            fflush(stdout);
            break;
        }
    }
}
