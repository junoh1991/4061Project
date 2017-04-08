#include <unistd.h>
#include <stdio.h>
#include "mm.h"

int main()
{
    mm_t mm;
    int i; 
    struct timeval time_s, time_e;
    void* test[NUM_CHUNKS];

    mm_init(&mm, NUM_CHUNKS, CHUNK_SIZE);
    
    gettimeofday(&time_s, NULL);
    for (i = 0; i <NUM_CHUNKS; i++)
        test[i] = mm_get(&mm);
    for (i = 0; i <NUM_CHUNKS; i++)
        mm_put(&mm, test[i]);
    gettimeofday(&time_e, NULL);

    fprintf(stderr, "Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);
    gettimeofday(&time_s, NULL);
    for (i = 0; i < NUM_CHUNKS; i++)
        test[i] = malloc(sizeof(CHUNK_SIZE));
    for (i = 0; i < NUM_CHUNKS; i++)
        free(test[i]);
    gettimeofday(&time_e, NULL);

    
    fprintf(stderr, "Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);

    mm_release(&mm);
    return 0;
}
