#include <unistd.h>
#include <stdio.h>
#include "mm.h"

int main()
{
    mm_t mm;
    int i; 

    mm_init(&mm, NUM_CHUNKS,CHUNK_SIZE);


    void *temp = mm_get(&mm); 
    printf("chunk address: %p\n", temp);
    

    temp = mm_get(&mm); 
    printf("chunk address: %p\n", temp);

    mm_put(&mm, temp);
    
    temp = mm_get(&mm);
    printf("chunk struct address: %p\n", temp );
    
    temp = mm_get(&mm);
    printf("chunk struct address: %p\n", temp );
    mm_release(&mm);
    return 0;
}
