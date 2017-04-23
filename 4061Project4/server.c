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
        int             m_socket;
        char    m_szRequest[MAX_REQUEST_LENGTH];
} request_t;

void * dispatch(void * arg)
{
        return NULL;
}

void * worker(void * arg)
{
        return NULL;
}

int main(int argc, char **argv)
{
        //Error check first.
        if(argc != 6 && argc != 7)
        {
                printf("usage: %s port path num_dispatcher num_workers queue_length [cache_size]\n", argv[0]);
                return -1;
        }

        printf("Call init() first and make a dispather and worker threads\n");
        return 0;
}
