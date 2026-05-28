#ifndef BUILTINS_H
#define BUILTINS_H

// forward declaration to avoid circular includes
typedef struct shell shell;
typedef int (*BinFn) (char**, shell*); // function pointer to a char* taking function
// functiontable, for cleaner execution
typedef struct Builtin
{
    char *name;
    BinFn bin;
    
} Builtin;

typedef struct bin_Hashmap
{
    BinFn builtin;
} bin_Hashmap;


int shell_exit(char** dir, shell*);
int cd(char** dir, shell*);
int pwd(char** flags, shell*);
int type(char** text, shell*);
int history(char **text, shell*);

void hashmap_populate(bin_Hashmap *map, int map_size, Builtin *table);
int hashkey_calculate(char* string, int modulo_param);
BinFn hashmap_poll(bin_Hashmap *map, int map_size, char* function);
void hashmap_initialize(bin_Hashmap *map, int map_size);
#endif