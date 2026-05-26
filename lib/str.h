#ifndef STR_H
#define STR_H

#define PATH_MAX 4096 // my system doesnt find PATH_MAX in limits.h itll be defined locally here

void strcopy(char* origin, char* destination);
int nstrcomp(char*, char*);
int strcomp(char*, char*);
//int flagstrcomp(char*, char*, int* flag, int number);
void tlc(char*);
int parseStr(char*, char**);
int strcopySeg(char* origin, char* dest, int start, int end);
void findPrefix(char** arr, char* prefix, int length);
int strLen(char* text);
void delInStr(char* text, int idx);
void insertInStr(char* text, char c, int idx, int size);
void initStr(char* text, int c, int size);
int increaseCapac(char*** array, int* capac, int amt);
int allocStrCopy(char* origin, char** dest, int orig_size);
void cutFromEnd(char *string, char c, int string_size);
#endif