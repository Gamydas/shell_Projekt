#ifndef STR_H
#define STR_H
void strcopy(char* origin, char* destination);
int nstrcomp(char*, char*);
int strcomp(char*, char*);
//int flagstrcomp(char*, char*, int* flag, int number);
void tlc(char*);
int parseStr(char*, char**);
int strcopySeg(char* origin, char* dest, int start, int end);
void findPrefix(char (*arr)[2048], char* prefix, int width, int length);
int strLen(char* text);
void delInStr(char* text, int idx);
void insertInStr(char* text, char c, int idx, int size);
void initStr(char* text, int c, int size);
#endif