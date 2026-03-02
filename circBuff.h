#ifndef CIRCBUFF_H
#define CIRCBUFF_H

typedef struct
{
    // dim 1 is the index dim 2 is the saved string
    char buf[50][50];
    char(*head)[50];   // head will always be a live input "pointer"; it will always "point" to the newest not yet written entry of the buffer
    char(*latest)[50]; // will be needed for history
    // the char (*name)[X] syntax means this pointer points to an array (in this case a string) of the size 50, this will be important for the functions of this struct
    
} charCircBuff;


void initializeCircBuf(charCircBuff* bufr);
void add2Buf(charCircBuff* bufr, char* text);
void tempadd2Buf(charCircBuff* bufr, char* text);
void resetLatest(charCircBuff* bufr);
void fetchLatest(charCircBuff* bufr, char* dest);
void revFetchLatest(charCircBuff* bufr, char* dest);

#endif