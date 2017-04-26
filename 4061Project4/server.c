/* csci4061 S2016 Assignment 4 
* section: one_digit_number 
* date: mm/dd/yy 
* names: Name of each member of the team (for partners)
* UMN Internet ID, Student ID (xxxxxxxx, 4444444), (for partners)
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "util.h"

#define MAX_THREADS 100
#define MAX_QUEUE_SIZE 100
#define MAX_REQUEST_LENGTH 1024
//Structure for a single request.
typedef struct request
{
    int     m_socket;
    char    m_szRequest[MAX_REQUEST_LENGTH];
} request_t;

// Mutex and ring buffer declaration
pthread_mutex_t request_access = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t slot_availble = PTHREAD_COND_INITIALIZER;
pthread_cond_t request_availble = PTHREAD_COND_INITIALIZER;
int dispatcher_count;
int worker_count;
int numb_dispatcher, numb_worker;

void * dispatch(void * arg)
{
    while(1)
    {
        pthread_mutex_lock(&request_access);
        while (dispatcher_count == numb_dispatcher)
            pthread_cond_wait(&slot_availble, &request_access);
        
        
    }
    return NULL;
}

void * worker(void * arg)
{
    return NULL;
}

int main(int argc, char **argv)
{
    int port;
    int i;
    pthread_t dispatcher_threads[numb_dispatcher];
    pthread_t worker_threads[numb_worker];

    //Error check first.
    if(argc != 6 && argc != 7)
    {
        printf("usage: %s port path num_dispatcher num_workers queue_length [cache_size]\n", argv[0]);
        return -1;
    }
    // Parse the arguments when there is no error to the arguments.
    port = atoi(argv[1]);
    numb_dispatcher = atoi(argv[3]);
    numb_worker = atoi(argv[4]);
    
    init(port);
    for(i = 0; i < numb_dispatcher; i ++) // create dispather threads;
        pthread_create(&dispatcher_threads[i], NULL, dispatch, NULL);
    for(i = 0; i < numb_worker; i ++)
        pthread_create(&worker_threads[i], NULL, worker, NULL);


    pause();
    printf("Call init() first and make a dispather and worker threads\n");
    return 0;
}
