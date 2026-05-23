#ifndef PIPELINING_H
#define PIPELINING_H

#include "redirect.h"
#include "parser.h"
#include "limits.h"

#define PARENT_PROCCESS INT_MAX // used as a return value to know which is the parent process when returning

int setUpPipes(Instructions *instruct, int size);
void closePipes(int pipes[][2], int size);


#endif