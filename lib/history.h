#ifndef HISTORY_H
#define HISTORY_H
#include <stdint.h>



typedef struct shHist
{
  struct shHist *next;
  struct shHist *prev;
  char* entry; 
  uint16_t entry_size;
  uint16_t entry_ID;

} shHist; 


int create_and_append_new_hist_entry(char* new_entry, uint16_t entry_size);
shHist* find_in_history(uint16_t entry_ID);
void delete_from_history(uint16_t entry_ID);
void free_history_entry(shHist* entry);
int read_history_from_file();
int write_history_to_file();
void print_history();
void clear_shell_history();








#endif
