
#include "packet.h"

int msqid = -1;

static message_t message;   /* current message structure */
static mm_t mm;             /* memory manager will allocate memory for packets */
static int pkt_cnt = 0;     /* how many packets have arrived for current message */
static int pkt_total = 1;   /* how many packets to be received for the message */

/*
   Handles the incoming packet. 
   Store the packet in a chunk from memory manager.
   The packets for given message will come out of order. 
   Hence you need to take care to store and assemble it correctly.
   Example, message "aaabbb" can come as bbb -> aaa, hence, you need to assemble it
   as aaabbb.
   Hint: "which" field in the packet will be useful.
 */
static void packet_handler(int sig) {
  packet_t pkt;
  void * chunk;
      
  // TODO get the "packet_queue_msg" from the queue.
  packet_queue_msg msg;
  msgrcv(msqid, (void *) &msg, sizeof(packet_t), QUEUE_MSG_TYPE, 0);
  // TODO extract the packet from "packet_queue_msg" and store it in the memory from memory manager
  pkt = msg.pkt;
  printf("%d, %d, %s\n", pkt.how_many, pkt.which, pkt.data);
  chunk = mm_get(&mm);
  memcpy(chunk, (void *) &pkt, sizeof(pkt));
  message.data[message.num_packets++] = chunk;
}

/*
 * TODO - Create message from packets ... deallocate packets.
 * Return a pointer to the message on success, or NULL
 */
static char *assemble_message() {

  void * msg;
  int i;
  int msg_len = message.num_packets * sizeof(data_t);

  /* TODO - Allocate msg and assemble packets into it */
  msg = (void *) malloc(msg_len);
  packet_t * temp;
  for(i = 0; i < message.num_packets; i++) {
    temp = (packet_t *) message.data[i];
    memcpy(msg + ((temp->which) * sizeof(data_t)), (void *) &temp->data, sizeof(data_t));
  }
  
  /* reset these for next message */
  pkt_total = 1;
  pkt_cnt = 0;
  message.num_packets = 0;

  return msg;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: packet_sender <num of messages to receive>\n");
    exit(-1);
  }

  int k = atoi(argv[1]); /* no of messages you will get from the sender */
  int i;
  char *msg;

  /* TODO - init memory manager for NUM_CHUNKS chunks of size CHUNK_SIZE each */
  mm_init(&mm, NUM_CHUNKS, CHUNK_SIZE);

  message.num_packets = 0;

  /* TODO initialize msqid to send pid and receive messages from the message queue. Use the key in packet.h */
  msqid = msgget(key, 0666 | IPC_CREAT);
  
  /* TODO send process pid to the sender on the queue */
  int pid = getpid();
  pid_queue_msg pid_msg;
  pid_msg.pid = pid;
  pid_msg.mtype = QUEUE_MSG_TYPE;
  msgsnd(msqid, (void *) &pid_msg, sizeof(int), 0);
  
  /* TODO set up SIGIO handler to read incoming packets from the queue. Check packet_handler()*/
  struct sigaction act;
  act.sa_handler = packet_handler;
  sigfillset(&act.sa_mask);
  sigaction(SIGIO, &act, NULL);

  for (i = 1; i <= k; i++) {
    while (pkt_cnt < pkt_total) {
      pause(); /* block until next packet */
    }
  
    msg = (char *) assemble_message();
    if (msg == NULL) {
      perror("Failed to assemble message");
    }
    else {
      fprintf(stderr, "GOT IT: message=%s\n", msg);
      free(msg);
    }
  }

  // TODO deallocate memory manager
  mm_release(&mm);

  // TODO remove the queue once done
  msgctl(msqid, IPC_RMID, 0);
  
  return EXIT_SUCCESS;
}
