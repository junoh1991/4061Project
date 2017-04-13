/* CSci4061 S2017 Assignment 3
* login: ohxxx371
* date: 4/12/2017
* name: Tristan Mansfield, Marcus Jun Oh
* id: mansf043, ohxxx371 */
#include <time.h>
#include "packet.h"

static int pkt_cnt = 0;       /* how many packets have been sent for current message */
static int pkt_total = 1;     /* how many packets to send send for the message */
static int msqid = -1;        /* id of the message queue */
static int receiver_pid;      /* pid of the receiver */
static int terminateFlag = 0; /* set to 1 if program needs to terminate */

/*
   Returns the packet for the current message. The packet is selected randomly.
   The number of packets for the current message are decided randomly.
   Each packet has a how_many field which denotes the number of packets in the current message.
   Each packet is string of 3 characters. All 3 characters for given packet are the same.
   For example, the message with 3 packets will be aaabbbccc. But these packets will be sent out order.
   So, message aaabbbccc can be sent as bbb -> aaa -> ccc
   */
static packet_t get_packet() {
    static int which = -1;
    static int how_many;
    static int num_of_packets_sent = 0;
    static int is_packet_sent[MAX_PACKETS];
    int i;

    packet_t pkt;
    if (num_of_packets_sent == 0) {
        how_many = rand() % MAX_PACKETS;
        if (how_many == 0) {
            how_many = 1;
        }
        pkt_total = how_many;
        printf("Number of packets in current message: %d\n", how_many);
        which = -1;
        for (i = 0; i < MAX_PACKETS; ++i) {
            is_packet_sent[i] = 0;
        }
    }
    which = rand() % how_many;
    if (is_packet_sent[which] == 1) {
        i = (which + 1) % how_many;
        while (i != which) {
            if (is_packet_sent[i] == 0) {
                which = i;
                break;
            }
            i = (i + 1) % how_many;
        } 
    }
    pkt.how_many = how_many;
    pkt.which = which;

    memset(pkt.data, 'a' + which, sizeof(data_t));

    is_packet_sent[which] = 1;
    num_of_packets_sent++;
    if (num_of_packets_sent == how_many) {
        num_of_packets_sent = 0;
    }

    return pkt;
}

/*
   Handles a SIGINT signal that would cause the program to terminate.
   Causes a SIGINT signal to be sent to the packet_receiver before termination.
   */
static void sigint_handler(int sig){
    terminateFlag = 1; 
}

/*
   Handles the SIGALRM signal from the timer that tells the program to send a packet
   to the packet_receiver process. The contents of the packet are printed to STDOUT,
   the packet is send via message queue to packet_receiver, and a SIGIO signal
   is sent to packet_receiver.
   */
static void packet_sender(int sig) {
    packet_t pkt;
    packet_queue_msg msg;

    pkt = get_packet();
    // temp is just used for temporarily printing the packet.
    char temp[PACKET_SIZE + 2];
    strcpy(temp, pkt.data);
    temp[3] = '\0';
    printf ("Sending packet: %s\n", temp);
    pkt_cnt++;
  
    // Create a packet_queue_msg for the current packet.
    msg.mtype = QUEUE_MSG_TYPE;
    msg.pkt = pkt;
    
    // send this packet_queue_msg to the receiver. Handle any error appropriately.
    if(msgsnd(msqid, (void *) &msg, sizeof(packet_t), 0) == -1) {
        perror("Message send failed");
        terminateFlag = 1;
        return;
    }
  
    // send SIGIO to the receiver if message sending was successful.
    kill(receiver_pid, SIGIO);
}

/*
   The packet_sender main function initializes the message queue, signal handlers,
   and timer, then runs the loops that keep track of how many messages and packets
   need to be sent. If the terminate flag is set, the process will send a SIGINT
   signal to packet_receiver before exiting.
   */
int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Usage: packet_sender <num of messages to send>\n");
        exit(-1);
    }

    int k = atoi(argv[1]); /* number of messages  to send */
    srand(time(NULL)); /* seed for random number generator */

    int i;
    pid_queue_msg msg;

    struct itimerval interval;
    struct sigaction act, act2;           

    /* Create a message queue */ 
    msqid = msgget(key, 0666 | IPC_CREAT);
  
    /* read the receiver pid from the queue and store it for future use*/
    printf("Waiting for receiver pid.\n");
    
    msgrcv(msqid, (void *) &msg, sizeof (int), QUEUE_MSG_TYPE, 0);
    receiver_pid = msg.pid;
    printf("Got pid : %d\n", receiver_pid);
   
    /* set up alarm handler -- mask all signals within it */
    /* The alarm handler will get the packet and send the packet to the receiver. Check packet_sender();
     * Don't care about the old mask, and SIGALRM will be blocked for us anyway,
     * but we want to make sure act is properly initialized.
     */
    act.sa_handler = packet_sender;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGINT);
    sigaction(SIGALRM, &act, NULL);
    
    // set up handler for SIGINT
    act2.sa_handler = sigint_handler;
    sigfillset(&act2.sa_mask);
    sigaction(SIGINT, &act2, NULL);
    /*  
     * turn on alarm timer ...
     * use  INTERVAL and INTERVAL_USEC for sec and usec values
    */
    interval.it_value.tv_sec = INTERVAL;
    interval.it_value.tv_usec = INTERVAL_USEC;
    interval.it_interval.tv_sec = 0;
    interval.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &interval, NULL);

    /* send k messages */
    for (i = 1; i <= k; i++) {
        printf("==========================%d\n", i);
        printf("Sending Message: %d\n", i);
        /* send packet pkt_total packets */
        while (pkt_cnt < pkt_total) {
            pause(); /* block until next packet is sent. SIGALARM will unblock and call the handler.*/
            // if SIGINT is received, exit program and send SIGINT to receiver process
            if (terminateFlag)
            {
                kill(receiver_pid, SIGINT);
                printf("Sender Exiting\n");
                exit(0); 
            }
            setitimer(ITIMER_REAL, &interval, NULL);
        }
        pkt_cnt = 0;
    }

    return EXIT_SUCCESS;
}
