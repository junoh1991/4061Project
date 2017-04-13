/* CSci4061 S2017 Assignment 3
* login: ohxxx371
* date: 4/12/2017
* name: Tristan Mansfield, Marcus Jun Oh
* id: mansf043, ohxxx371 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "mm.h"

int main()
{
    int i; 
    struct timeval time_s, time_e;
    char* test[NUM_CHUNKS];

    gettimeofday(&time_s, NULL);
    for (i = 0; i < NUM_CHUNKS; i++)
        test[i] = (char*) malloc(sizeof(CHUNK_SIZE));
    for (i = 0; i < NUM_CHUNKS; i++)
        free(test[i]);
    gettimeofday(&time_e, NULL);
    fprintf(stderr, "Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);

    return 0;
}
