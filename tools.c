#include <stdio.h>
#include <stdlib.h>

/// @brief doubles the capacity of char** i.e a sting array 
/// @param array 
/// @param capac 
/// @return returns pointer to realloced memory, NULL if realloc failed. Caller has to run a NULL check
char** doubleCapacity(char** array, int* capac)
{
    
    char** temp = realloc(array, (2 * (*capac)) * sizeof(char*));
    if(temp != NULL)
    {
        (*capac) *= 2; // doubles given capacity on success
    }
    return temp;

}


/// @brief function to find the minimal value in an array
/// @param array 
/// @param size size of array
/// @return min value 
int findMin(int* array, int size)
{
   
    if(array == NULL)
    {
        fprintf(stderr,"Array is not initialized\n");
        return -1;
    }

    int min = array[0];
   

    for (int i = 0; i < size; i++)  
    {
        if(*(array + i) < min)
        {
            min = *(array + i);
        }
    }
    return min;
}

/// @brief function to find the minimal value in an array, ignoring -1
/// @param array 
/// @param size size of array
/// @return min value 
int findMinXn1(int* array, int size)
{
   
    if(array == NULL)
    {
        fprintf(stderr,"Array is not initialized\n");
        return -1;
    }

    int min = __INT_MAX__;
   

    for (int i = 0; i < size; i++)  
    {
        if(*(array + i) < min && *(array + i) != -1)
        {
            min = *(array + i);
        }
    }
    return min;
}


/// @brief function to find the maximal value in an array
/// @param array 
/// @param size size of array
/// @return max value 
int findMax(int* array, int size)
{
   
    if(array == NULL)
    {
        fprintf(stderr,"Array is not initialized\n");
        return -1;
    }

    int max = -__INT_MAX__;
   

    for (int i = 0; i < size; i++)  
    {
        if(*(array + i) > max)
        {
            max = *(array + i);
        }
    }
    return max;
}