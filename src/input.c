#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include "escapesequenzen.h"
#include "shell.h"
#include "str.h"
#include "tab.h"
#include "history.h"

extern shHist* first_entry;
extern shHist* last_entry;

/// @brief initializes struct of rawInput type
/// @param input
/// @return 0 for success, -1 for error
int initialize_rawinput(rawInput* input)
{
    input->capac = 2048;  // base capacity of input string, will be doubled if needed
    input->cursoridx = 0;
    input->cmd = calloc(1, input->capac);
    if (input->cmd == NULL)
    {
        perror("malloc");
        return -1;
    }
    return 0;
}

/// @brief frees an object of type rawInput
/// @param input
void free_rawinput(rawInput* input)
{
    free(input->cmd);
}

/// @brief function to handle escapesequences, mainly the arrow keys for history navigation
/// @param sh
/// @param cmd_
/// @return 0 if success, -1 if an error occured
int handle_arrows(shell* sh, rawInput* cmd_)
{
    int cntrl = 0;
    char c = '\0';
    int rd = read(0, &c, 1);
    if (rd < 0)  // checks failed read
    {
        perror("read");
        return -1;
    }
    else if (rd == 0)  // checks end of file
    {
        return -1;
    }

    if (c != '[')  // all arrow keys begin with \033[
    {
        return 0;  // there are escape sequences that dont follow with [ so no -1
    }
    else
    {
        int rd = read(0, &c, 1);
        if (rd < 0)  // checks failed read
        {
            perror("read");
            return -1;
        }
        else if (rd == 0)  // checks end of file
        {
            return -1;
        }

        switch (c)
        {
            case 'A':  // arrow up
                // ID of oldest entry & check for empty hist
                if (sh->histpos <= 1)
                {
                    printf("\a");
                    fflush(stdout);
                    break;
                } /* checks if oldest command has been reached*/
                // stores current input in history
                cntrl = shHist_modify(cmd_->cmd, sh->histpos);
                if (cntrl < 0) return -1;
                // repositioning history Index
                sh->histpos--;
                // copying stored instruction into current input 
                str_copy(find_in_history(sh->histpos)->entry, cmd_->cmd);
                // repositioning cursoridx
                cmd_->cursoridx = str_len(cmd_->cmd) + str_len(sh->wdir) + 2;  
                break;

            case 'B':  // arrow down
                if (last_entry == NULL ||  sh->histpos >= last_entry->entry_ID)
                {
                    printf("\a");
                    fflush(stdout);
                    break;
                } 

                // stores current input in history
                cntrl = shHist_modify(cmd_->cmd, sh->histpos);
                if (cntrl < 0) return -1;
                // repositioning cursoridx
                sh->histpos++;
                // copying stored instruction into current input 
                str_copy(find_in_history(sh->histpos)->entry, cmd_->cmd);  
                // repositioning cursoridx
                cmd_->cursoridx = str_len(cmd_->cmd) + str_len(sh->wdir) + 2;  
                break;
            case 'C':
                // checks if the curosr has met the rightmost edge of the command then increments cursor if not
                if (cmd_->cursoridx < str_len(cmd_->cmd) + str_len(sh->wdir) + 2)
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
                if (cmd_->cursoridx > str_len(sh->wdir) + 2)
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
            default:  // other escape sequences starting with \033[ are ignored for now
                break;
        }
    }
    return 0;
}

/// @brief function to reposition cursor on the screen
/// @param cursoridx value that indicates where the curoser is currently positioned
void reposition_cursor(int cursoridx)
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
int get_input(shell* sh, rawInput* cmd_)
{
    tabComp tab;
    // at this point only these 2 fields are relevant to initialize, remaining fields get initialized when needed
    tab.tabs = 0;
    tab.matches = NULL;
    int len = 0;                     // variable checks how long the currently typed cmd is
    int anc = str_len(sh->wdir) + 2;  // varible to memorize the beggining of the user editable part of the cmd line
    char c = '\0';
    cmd_->cursoridx = anc;            // positions cursor at the first user eligable position
    tcsetattr(0, TCSANOW, &sh->raw);  // enters raw mode


    printf("%s: ", sh->wdir);  // display prompt
    fflush(stdout);

    while (1)
    {
        int rd = read(0, &c, 1);
        if (rd < 0)  // checks failed read
        {
            perror("read");
            tcsetattr(0, TCSANOW, &sh->canon);  // return to canon mode
            return -1;
        }
        else if (rd == 0)  // checks end of file
        {
            tcsetattr(0, TCSANOW, &sh->canon);  // return to canon mode
            return -1;
        }
        switch (c)
        {
            case '\t':  // Tabulator to autocomplete the directory if possible
                if (tab.tabs == 0)
                {
                    initialize_tab_struct(&tab);
                }
                tab.tabs++;
                int cntrl = tab_completion(&tab, sh->builtins, sh->binamt, cmd_->cmd, sh->wdir);
                if (cntrl < 0)
                {
                    continue;
                }
                // refreshes screen
                printf("\r\033[K%s: %s", sh->wdir, cmd_->cmd);
                fflush(stdout);

                len = str_len(cmd_->cmd);      // readjusts str len
                cmd_->cursoridx = len + anc;  // readjusts cursor
                break;

            case '\n':  // user pressed enter to send their instruction
                if (cmd_->cmd[0] == '\0')
                {
                    printf("\a");
                    fflush(stdout);
                    continue;
                }
                // appends current input into history
                create_and_append_new_hist_entry(cmd_->cmd, str_len(cmd_->cmd));
                // positions history Index after the newest command
                sh->histpos = last_entry->entry_ID + 1;
                cleanup_tab_struct(&tab);  // avoid memory leaks
                printf("\r\n");
                fflush(stdout);
                tcsetattr(0, TCSANOW, &sh->canon);  // exits raw mode

                /*
            if ((addItem(&sh->history, sh->cmd)) == 0) // checks if adding item was successful
            {
                sh->latest = sh->history.size - 1;     // resets latest
            }
            */
                return str_len(cmd_->cmd);
            case 127:
                cleanup_tab_struct(&tab);  // avoid memory leaks
                if (cmd_->cursoridx > anc)
                {
                    len--;
                    cmd_->cursoridx--;
                    delete_in_string(cmd_->cmd, cmd_->cursoridx - anc);  // removes item in string

                    // refreshes the screen
                    printf("\r\033[K%s: %s", sh->wdir, cmd_->cmd);
                    fflush(stdout);
                    reposition_cursor(cmd_->cursoridx);
                }
                else
                {
                    printf("\a");
                    fflush(stdout);
                }

                break;

            case '\033':  // checks for all inputs starting with \033 mainly for arrow keys, ignoring every other instruction that begins like this, might add more later
                if (handle_arrows(sh, cmd_) < 0)
                {
                    tcsetattr(0, TCSANOW, &sh->canon);  // return to canon mode
                    return -1;
                }
                // refreshes the screen
                printf("\r\033[K%s: %s", sh->wdir, cmd_->cmd);
                fflush(stdout);
                reposition_cursor(cmd_->cursoridx);

                len = str_len(cmd_->cmd);
                cleanup_tab_struct(&tab);  // avoid memory leaks

                break;
            default:
                // checks if size of cmd string needs to be increased
                if (len == cmd_->capac)
                {
                    cmd_->capac *= 2;  // doubles capacity
                    char* temp = realloc(cmd_->cmd, cmd_->capac);
                    if (temp == NULL)
                    {
                        perror("malloc");
                        tcsetattr(0, TCSANOW, &sh->canon);  // return to canon mode
                        return -1;
                    }

                    cmd_->cmd = temp;
                }
                // cursor is in the string not at the end
                if (cmd_->cursoridx - anc != len)
                {
                    insert_in_string(cmd_->cmd, c, cmd_->cursoridx - anc, str_len(cmd_->cmd));
                    cmd_->cursoridx++;
                    len++;
                    cmd_->cmd[len] = '\0';
                }
                else
                {  // cursor is at the end of the string
                    cmd_->cmd[cmd_->cursoridx - anc] = c;
                    cmd_->cursoridx++;
                    len++;
                    cmd_->cmd[len] = '\0';
                }

                // refreshing screen
                printf("\r\033[K%s: %s", sh->wdir, cmd_->cmd);
                fflush(stdout);
                reposition_cursor(cmd_->cursoridx);

                cleanup_tab_struct(&tab);  // avoid memory leaks
                break;
        }
    }
}
