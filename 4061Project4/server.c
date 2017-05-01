/* csci4061 S2016 Assignment 4 
* date: 05/03/2017
* names: Tristan Mansfield, mansf043, 5065831, section 11
         Marcus Jun Oh, ohxxx371, 5214365, section 13
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "util.h"
#include "unistd.h"
#include <errno.h>

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
pthread_mutex_t file_access = PTHREAD_MUTEX_INITIALIZER;


int dispatcher_count;
int worker_count;
int request_count;
int numb_dispatcher, numb_worker;
int ringsize;
char* root_dir;
request_t request_array[MAX_QUEUE_SIZE];
FILE* fp;

// Dispatcher threads run until a request is found, then sends it to a worker thread to complete.
void * dispatch(void * arg)
{
    int fd;
    while(1)
    {
        // block until request is found
        if ((fd = accept_connection()) < 0)
            continue;
            
        // If request is found, lock mutex, increase bounded buffer, signal dispatcher threads,
        // and unlock mutex for next connection.
        pthread_mutex_lock(&request_access);
        while (request_count == ringsize)
            pthread_cond_wait(&slot_availble, &request_access);
        request_array[dispatcher_count].m_socket = fd;
        if (get_request(fd, request_array[dispatcher_count].m_szRequest) < 0)
        {
            printf("Faulty request. Resuming.\n");
            pthread_mutex_unlock(&request_access);
            continue;
        }
        dispatcher_count = (dispatcher_count + 1) % ringsize;
        request_count++;
        pthread_cond_signal(&request_availble);
        pthread_mutex_unlock(&request_access);
    }
    return NULL;
}

// Worker threads receive a request from the dispatcher threads, then get the requested file (or handles
// an error on a faulty request.
// Note: Each thread is assigned a thread ID from the main function (as void * arg)
void * worker(void * arg)
{
    int fd;
    int in;
    char *request;
    char *full_path;
    char *content_type;
    int read_count;
    int file_size;
    int index;
    int reg_num = 0;
    int threadID = *(int*)arg;
    while(1)
    {
        // When dispatcher thread signals this thread, block mutex, decrease bounded buffer, 
        // signals dispatcher thread, and unlock mutex for next worker threads.
        pthread_mutex_lock(&request_access);
        while (request_count == 0)
            pthread_cond_wait(&request_availble, &request_access);
        request = request_array[worker_count].m_szRequest;
        fd = request_array[worker_count].m_socket;
        request_count--;
        worker_count = (worker_count + 1) % ringsize;
        pthread_cond_signal(&slot_availble);
        pthread_mutex_unlock(&request_access);
        // Parse full path from request
        reg_num++;
        char full_path[sizeof(root_dir) + sizeof(request) +1];
        strcpy(full_path, root_dir);
        strcat(full_path, request);
        // Attempt to open file from full path
        in = open(full_path, O_RDONLY);
        if (in == -1) // File is not found, return error and write error to log
        {
            return_error(fd, "Bad request");
            pthread_mutex_lock(&file_access);
            fprintf(fp, "[%d][%d][%d][%s][File not found.]\n", threadID, reg_num, fd, request);
            fflush(fp);
            pthread_mutex_unlock(&file_access);
        }
        else // File is found, determine file type and size, read file to buffer, return result to client
             // and write request info to log
        {
            if (strstr(full_path, ".html")) 
                content_type = "text/html";
            else if (strstr(full_path, ".gif"))
                content_type = "image/gif";
            else if (strstr(full_path, ".jpg"))
                content_type = "image/jpeg";
            else
                content_type = "text/plain";        

            file_size = lseek(in, 0, SEEK_END);
            lseek(in, 0, SEEK_SET);
            
            char transmit_buffer[file_size];
            read(in, transmit_buffer, file_size);
            close(in);
            
            if (return_result(fd, content_type, transmit_buffer, file_size) != 0) 
                printf("Error occured returning the requested file to the clinet\n");
            
            pthread_mutex_lock(&file_access);
            fprintf(fp, "[%d][%d][%d][%s][%d]\n", threadID, reg_num, fd, request, file_size);
            fflush(fp);
            pthread_mutex_unlock(&file_access);
        }
    }    

    return NULL;
}

// Initializes the web server and creates dispatcher and worker threads, runs until manually terminated
// or ran out of threads. 
int main(int argc, char **argv)
{
    int port;
    int i;

    //Error check first.
    if(argc != 6 && argc != 7)
    {
        printf("usage: %s port path num_dispatcher num_workers queue_length [cache_size]\n", argv[0]);
        return -1;
    }
    // Parse the arguments when there is no error to the arguments.
    port = atoi(argv[1]);
    root_dir = argv[2];
    numb_dispatcher = atoi(argv[3]);
    numb_worker = atoi(argv[4]);
    ringsize = atoi(argv[5]);
    
    if (numb_dispatcher > MAX_THREADS || numb_worker > MAX_THREADS || ringsize > MAX_QUEUE_SIZE ||
        numb_dispatcher < 1 || numb_worker < 1 || ringsize < 1)
    {
        printf("Number of dispatcher threads, worker threads, and queue length must all be between 1 and 100 each.\n");
        exit(-1);
    }
    
    request_count = 0;
    dispatcher_count = 0;
    worker_count = 0;
    pthread_t dispatcher_threads[numb_dispatcher];
    pthread_t worker_threads[numb_worker];
    int threadID[numb_worker];
    // create thread ids
    for (i = 0; i < numb_worker; i++)
    {
        threadID[i] = i;
    }

    if ((fp = fopen("./webserver_log", "w+")) == NULL) {
        printf("Log could not be created.\n");
        exit(-1);
    }

    init(port);
    // Initialze all threads according to the parameters passed into the main.
    for(i = 0; i < numb_dispatcher; i ++) // create dispather threads;
        pthread_create(&dispatcher_threads[i], NULL, dispatch, NULL);
    for(i = 0; i < numb_worker; i ++) // create worker threads
        pthread_create(&worker_threads[i], NULL, worker, (void*)&threadID[i]);

    // Wait until all threads are depleted. Ideally, no threads should be closed.
    for(i = 0; i < numb_dispatcher; i ++)
        pthread_join(dispatcher_threads[i], NULL);
    for(i = 0; i < numb_worker; i ++)
        pthread_join(worker_threads[i], NULL);

    // Program ran out of threads. Close the log file.
    fclose(fp);
    return 0;
}
