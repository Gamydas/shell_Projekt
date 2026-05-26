#ifndef STR_H
#define STR_H

#define PATH_MAX 4096 // my system doesnt find PATH_MAX in limits.h itll be defined locally here

void str_copy(char* origin, char* destination);
int str_comp(char*, char*);
//int flagstr_comp(char*, char*, int* flag, int number);
void to_lower_case(char*);
int parse_string(char*, char**);
int segment_str_copy(char* origin, char* dest, int start, int end);
void find_prefix(char** arr, char* prefix, int length);
int str_len(char* text);
void delete_in_string(char* text, int idx);
void insert_in_string(char* text, char c, int idx, int size);
void initialize_string(char* text, int c, int size);
int increase_capacity(char*** array, int* capac, int amt);
int alloc_str_copy(char* origin, char** dest, int orig_size);
void cut_character_from_end(char *string, char c, int string_size);
#endif