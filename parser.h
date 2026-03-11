#ifndef PARSER_H
#define PARSER_H

typedef enum
{
    SINGLE_QUOTES,
    DOUBLE_QUOTES,
    NORMAL

} MODUS;

void parseInput(char* text, char** parsed);
void switchModes(MODUS *mode, char c);
#endif