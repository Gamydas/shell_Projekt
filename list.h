#ifndef LIST_H
#define LIST_H

typedef struct
{
    char (*list)[2048];
    int size;
} list;

void initList(list* list_);
int addItem(list* list_, char* text);
char* fetchItem(list* list_, int* idx);
void freeList(list* list_);
// void rmItem(list* list_);


#endif