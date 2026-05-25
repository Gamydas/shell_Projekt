#ifndef HISTORY_H
#define HISTORY_H




typedef struct shHist
{
  struct shHist *next;
  struct shHist *prev;
  char* entry;
  int entry_size;
  int entry_ID;

} shHist; 


void create_new_hist_entry(char* new_entry, int entry_s);











#endif
