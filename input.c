#include <stdio.h>
#include "escapesequenzen.h"
/// @brief function lets the user input a string which will be interpreted as a command
/// @param Prompt giving prompt by calling function, i.e. cwd 
/// @param cmd string which will be interpreted as a command
void getInput(char* Prompt, char* cmd)              
{
    printf("%s: ", Prompt);                         // Prompt auf Bildschirm ausgeben
    *cmd = '\0';                                    // Zur Kontrolle falls Benutzer nichts eingibt
    do
    {
        scanf("%200[^\n]", cmd);
        
        while(getchar()!='\n');                    // emptying Input Buffer

        /*
        if(*cmd)                                     
        {
            UP_LINE;
            printf("%s\n", cmd);
        }
        */
    
    } while (!*cmd);
    
    
}
