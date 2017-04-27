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
int dispatcher_count;
int worker_count;
int request_count;
int numb_dispatcher, numb_worker;
int ringsize;
char* root_dir;
request_t request_array[MAX_QUEUE_SIZE];

void * dispatch(void * arg)
{
    int fd;
    while(1)
    {
        if ((fd = accept_connection()) < 0)
        {
            //pthread_mutex_unlock(&request_access);
            continue;
        }

        
        pthread_mutex_lock(&request_access);
        while (request_count == ringsize)
            pthread_cond_wait(&slot_availble, &request_access);
        request_array[dispatcher_count].m_socket = fd;
        if (get_request(fd, request_array[dispatcher_count].m_szRequest) < 0)
        {
            pthread_mutex_unlock(&request_access);
            continue;
        }
        printf("%s\n", request_array[dispatcher_count].m_szRequest);
        dispatcher_count = (dispatcher_count + 1) % ringsize;
        request_count++;
        pthread_cond_signal(&request_availble);
        pthread_mutex_unlock(&request_access);
    }
    return NULL;
}

void * worker(void * arg)
{
    int fd;
    int in;
    char *request;
    char *full_path;
    char *content_type;
    int read_count;
    int file_size;
    char error_message[100];
    int index;
    int reg_num = 0;
    FILE* fp;
    while(1)
    {
        pthread_mutex_lock(&request_access);
        while (request_count == 0)
            pthread_cond_wait(&request_availble, &request_access);
        request = request_array[worker_count].m_szRequest;
        fd = request_array[worker_count].m_socket;
        request_count--;
        worker_count = (worker_count + 1) % ringsize;
        pthread_cond_signal(&slot_availble);
        pthread_mutex_unlock(&request_access);
        reg_num++;
        
        full_path = (char *) malloc(sizeof(root_dir) + sizeof(request) + 1);
        strcpy(full_path, root_dir);
        strcat(full_path, request);
        
        in = open(full_path, O_RDONLY);
        if (in == -1)
            return_error(fd, error_message);
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
        printf("%s\n", transmit_buffer);
        close(in);
        
        return_result(fd, content_type, transmit_buffer, file_size);
        
        pthread_mutex_lock(&request_access);
        //log_fd = open("./webserver_log.txt", O_WRONLY | O_APPEND);
        //write(log_fd, "derp\n", 5);
        fp = fopen("./webserver_log.txt", "a");
        fprintf(fp, "[%d][%d][%d][%s][%d]\n", (int) pthread_self(), reg_num, fd, request, file_size);
        fclose(fp);
        //close(log_fd);
        pthread_mutex_unlock(&request_access);
    }    

    return NULL;
}

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
    request_count = 0;
    dispatcher_count = 0;
    worker_count = 0;
    pthread_t dispatcher_threads[numb_dispatcher];
    pthread_t worker_threads[numb_worker];

    //if ((log_fd = open("./webserver_log.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) == -1) {
    //    printf("Log could not be created.\n");
    //    exit(-1);
    //}
    //close(log_fd);
    FILE* fp;
    if ((fp = fopen("./webserver_log.txt", "w+")) == NULL) {
        printf("Log could not be created.\n");
        exit(-1);
    }
    fclose(fp);

    init(port);
    for(i = 0; i < numb_dispatcher; i ++) // create dispather threads;
        pthread_create(&dispatcher_threads[i], NULL, dispatch, NULL);
    for(i = 0; i < numb_worker; i ++)
        pthread_create(&worker_threads[i], NULL, worker, NULL);


    pause();
    printf("Call init() first and make a dispather and worker threads\n");
    return 0;
}
