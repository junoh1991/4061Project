#include <stdio.h>
#include <stdlib.h>
#include "mm.h"
//#define DEBUG


/* Return usec */
double comp_time(struct timeval time_s, struct timeval time_e) {

  double elap = 0.0;

  if (time_e.tv_sec > time_s.tv_sec) {
    elap += (time_e.tv_sec - time_s.tv_sec - 1) * 1000000.0;
    elap += time_e.tv_usec + (1000000 - time_s.tv_usec);
  }
  else {
    elap = time_e.tv_usec - time_s.tv_usec;
  }
  return elap;
}
/* int mm_init(mm_t *mm, int hm, int sz) 
 * This function allocates a memory manager. The memory manager allocate a chunk of memory
 * , with each chunk equal to the size of sz and numbeer of chunks equal to the hm.
 * returns 0 on success.
 * returns -1 on failure. 
*/
int mm_init(mm_t *mm, int hm, int sz) {
    int i, j;

    // Allocate a head pointer which will track the 'put' memory 
    mm->headRef = (struct chunk_t*) malloc(sizeof(struct chunk_t));
    mm->headRef->next = NULL;

    // Allocate blocks of memory
    mm->lastIndex = 0;
    mm->chunk =(struct chunk_t*) malloc(sizeof(struct chunk_t) *hm);
    for (i = 0; i < hm; i++)
    {
        mm->chunk[i].arr =  malloc(sizeof(void) *sz);
        mm->chunk[i].status = (uint8_t*) malloc(sizeof(uint8_t));

        // If there are any issues with malloc, free all malloc'ed memory and return -1;
        if (mm->chunk[i].arr == NULL)
        {
            free(mm->chunk);
            free(mm->headRef);
            return -1;
        }
    }

    // Metadata for later functions
    mm->howMany = hm;
    mm->offset = (uintptr_t)mm->chunk[0].status - (uintptr_t)mm->chunk[0].arr;
 
#ifdef DEBUG
    printf("offset: %d\n", mm->offset);
    printf("Start Address: %p, End Address: %p\n", mm->chunk[0].arr, mm->chunk[--i].arr);
#endif     
    return 0;
}


/* void *mm_get(mm_t *mm) 
 * This functions returns an availble chunk of memory from the memory manager.
 * Returns a chunk when successful.
 * Returns NULL on failure.
*/
void *mm_get(mm_t *mm) {
    int index;
    struct chunk_t *temp;

    // Check released stack first.
    if ((mm->headRef->next) != NULL)
    {
        temp = mm->headRef->next;
        (mm->headRef->next) = temp->next;
        temp->next = NULL;
        *(temp->status) = TAKEN;
        return temp->arr;     
    }

    else // Stack is empty. Get memory from the initial pool.
    {
        index = mm->lastIndex;
        if (index >= NUM_CHUNKS)// No more memory left
            return NULL;
        *(mm->chunk[index].status) = TAKEN;
        mm->lastIndex++;
        return (void*) mm->chunk[index].arr; 
    }
}

/* void mm_put(mm_t *mm, void*chunk)
 * This function puts back the allocated memory from mm_get.
*/
void mm_put(mm_t *mm, void *chunk) {
    struct chunk_t* temp = (struct chunk_t*) chunk;
    int *status = chunk + mm->offset;
    
    // Check if the chunk is already Free
    if (*status == FREE)
        return;
    else
        *status = FREE;
    temp->arr = NULL;

    // If headRef's next poitner is null, we know the stack is empty
    if(mm->headRef->next != NULL)
        temp->next = mm->headRef->next;
    mm->headRef->next = temp; 
#ifdef DEBUG
    printf("freed chunk address: %p, \n", (mm->headRef->next));    
#endif
}

/* void mm_release(mm_t *mm)
 * This function released heaps created by mm_init()
*/
void mm_release(mm_t *mm) {
    int i;

    // Free chunk_t member variables.
    for (i = 0 ; i < mm->howMany; i ++)
    {
        free(mm->chunk[i].arr);
        free(mm->chunk[i].status);
    }
        
    // Free mm_t member variables.
    free(mm->chunk);
    free(mm->headRef);
}

/*
 * TODO - This is just an example of how to use the timer.  Notice that
 * this function is not included in mm_public.h, and it is defined as static,
 * so you cannot call it from other files.  Instead, just follow this model
 * and implement your own timing code where you need it.
 */
static void timer_example() {
  struct timeval time_s, time_e;

  /* start timer */
  gettimeofday (&time_s, NULL);

  /* TODO - code you wish to time goes here */

  gettimeofday(&time_e, NULL);

  fprintf(stderr, "Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);
}
