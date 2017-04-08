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

/* TODO - Implement.  Return 0 for success, or -1 and set errno on fail. */
int mm_init(mm_t *mm, int hm, int sz) {
    int i;

    // Allocate stack for 'put' memory
    mm->headRef = (struct chunk_t*) malloc(sizeof(struct chunk_t));
    mm->headRef->next = NULL;

    // Allocate blocks of memory
    mm->lastIndex = 0;
    mm->chunk =(struct chunk_t*) malloc(sizeof(struct chunk_t) *hm);
    for (i = 0; i < hm; i++)
        mm->chunk[i].arr =  malloc(sizeof(void) *sz);
#ifdef DEBUG
    printf("Start Address: %p, End Address: %p\n", mm->chunk[0].arr, mm->chunk[--i].arr);
#endif     
    return 0;  /* TODO - return the right value */
}

/* TODO - Implement */
void *mm_get(mm_t *mm) {
    int index;
    struct chunk_t *temp;
    // Check released stack first.
    if ((mm->headRef->next) != NULL)
    {
        temp = mm->headRef->next;
        (mm->headRef->next) = temp->next;
        temp->next = NULL;
        return temp->arr;     
    }

    else
    {
        index = mm->lastIndex++;
        return (void*) mm->chunk[index].arr; 
    }
}

/* TODO - Implement */
void mm_put(mm_t *mm, void *chunk) {
    struct chunk_t* temp = (struct chunk_t*) chunk;
    if(mm->headRef->next != NULL)
        temp->next = mm->headRef->next;
    mm->headRef->next = temp; 
#ifdef DEBUG
    printf("freed chunk address: %p, \n", (mm->headRef->next));    
#endif
}

/* TODO - Implement */
void mm_release(mm_t *mm) {
    free(mm->chunk);
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
