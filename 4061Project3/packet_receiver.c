/* CSci4061 S2017 Assignment 3
* login: ohxxx371
* date: 4/12/2017
* name: Tristan Mansfield, Marcus Jun Oh
* id: mansf043, ohxxx371 */
#include "packet.h"

int msqid = -1;

static message_t message;   /* current message structure */
static mm_t mm;             /* memory manager will allocate memory for packets */
static int pkt_cnt = 0;     /* how many packets have arrived for current message */
static int pkt_total = 1;   /* how many packets to be received for the message */
static char *msg;           /* character string to hold the assembled message */
/*
   Handles the incoming packet. 
   Store the packet in a chunk from memory manager.
   The packets for given message will come out of order. 
   Hence you need to take care to store and assemble it correctly.
   Example, message "aaabbb" can come as bbb -> aaa, hence, you need to assemble it
   as aaabbb.
 */
static void packet_handler(int sig) {
    packet_t pkt;
    void * chunk;
    packet_queue_msg msg;
     
    // get the "packet_queue_msg" from the queue.
    msgrcv(msqid, (void *) &msg, sizeof(packet_t), QUEUE_MSG_TYPE, 0);
    // extract the packet from "packet_queue_msg" and store it in the memory from memory manager
    pkt = msg.pkt;
    chunk = mm_get(&mm);
    memcpy(chunk, (void *) &pkt, sizeof(pkt));
    message.data[message.num_packets++] = chunk;
    // update pkt_cnt and pkt_total
    pkt_cnt++;
    pkt_total = pkt.how_many;
}

/*
   Handles the SIGINT signal. Frees the memory used by the memory manager and closes
   the message queue before terminating the process.
 */
static void sigint_handler(int sig)
{
    mm_release(&mm);
    free(msg);
    msgctl(msqid, IPC_RMID, 0);
    printf("Receiver Exiting\n");
    exit(0);
} 

/*
   Create message from packets.
   Return a pointer to the message on success.
 */
static char* assemble_message() {
    char* msg;
    int i;
    packet_t * temp;
    int msg_len = message.num_packets * sizeof(data_t);

    /* Allocate msg and assemble packets into it */
    msg = (char*) malloc(msg_len+1);
    for(i = 0; i < message.num_packets; i++) {
        temp = (packet_t *) message.data[i];
        memcpy(msg + ((temp->which) * sizeof(data_t)), (void *) &temp->data, sizeof(data_t));
        //mm_put(&mm, (void *) temp);
    }
    msg[msg_len] = '\0';  
    
    /* reset these for next message */
    pkt_total = 1;
    pkt_cnt = 0;
    message.num_packets = 0;

    return msg;
}

/*
   The packet_receiver main function initializes the memory manager, message queue,
   and signal handlers, then runs the loops that keep track of how many messages and
   packets need to be received. When all of the packets of a message are received, the
   message is assembled and printed to STDOUT. When all messages have been received,
   the memory manager's memory is freed and the message queue is closed before the
   process terminates.
   */
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: packet_sender <num of messages to receive>\n");
        exit(-1);
    }

    int k = atoi(argv[1]); /* no of messages you will get from the sender */
    
    int i;
    pid_queue_msg pid_msg;
    int pid;
    
    struct sigaction act, act2;

    /* init memory manager for NUM_CHUNKS chunks of size CHUNK_SIZE each */
    mm_init(&mm, NUM_CHUNKS, CHUNK_SIZE);

    message.num_packets = 0;

    /* initialize msqid to send pid and receive messages from the message queue. Use the key in packet.h */
    msqid = msgget(key, 0666 | IPC_CREAT);
    
    /* send process pid to the sender on the queue */
    pid = getpid();
    pid_msg.pid = pid;
    pid_msg.mtype = QUEUE_MSG_TYPE;
    printf("Sending pid: %d\n", pid);
    msgsnd(msqid, (void *) &pid_msg, sizeof(int), 0);
    
    /* set up SIGIO handler to read incoming packets from the queue. Check packet_handler()*/ 
    act.sa_handler = packet_handler;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGINT);
    sigaction(SIGIO, &act, NULL);
    
    /* set up SIGINT handler to terminate the program if the packet_sender encounters an issues*/
    act2.sa_handler = sigint_handler;
    sigfillset(&act2.sa_mask);
    sigaction(SIGINT, &act2, NULL);
    
    // receive k messages
    for (i = 1; i <= k; i++) {
        // receive pkt_cnt packets
        while (pkt_cnt < pkt_total) {
            pause(); /* block until next packet */
        }
        
        msg =  assemble_message();
        if (msg == NULL) {
            perror("Failed to assemble message");
        }
        else {
            fprintf(stderr, "GOT IT: message=%s\n", msg);
            free(msg);
        }
    }

    // deallocate memory manager
    mm_release(&mm);

    // remove the queue once done
    msgctl(msqid, IPC_RMID, 0);
    
    return EXIT_SUCCESS;
}
