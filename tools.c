#include <stdio.h>
#include <stdlib.h>

/// @brief increases the capacity of char** i.e a sting array 
/// @param array 
/// @param capac 
/// @return 0 if success, -1 if error
int increaseCapacity(char** array, int* capac)
{
    (*capac) *= 2; // doubles given capacity
    char** temp = realloc(array, *capac * sizeof(char*));
    if (temp == NULL)
    {
        perror("realloc");
        return -1;
    }
    array = temp;
    return 0;

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