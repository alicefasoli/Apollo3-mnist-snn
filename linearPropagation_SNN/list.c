#include "list.h"
#include "neuron.h"

void printListInt(int *list, int len)
{
    printf("[ ");
    for(int i=0; i < len; i++){
        printf("%d ", list[i]);
    }
    printf("]\n");
}

void printListFloat(float *list, int len)
{
    printf("[ ");
    for(int i=0; i < len; i++){
        printf("%2.3f ", list[i]);
    }
    printf("]\n");    
}

void printOnlyWhenFire(float* list, int len){
    printf("\n");
    for(int i=0; i < len; i++){
        if(list[i] > 0.0){
            printf("Fire at position [%d] with value [%2.3f]\n", i, list[i]);
        }
        
    }
    printf("\n"); 
}

