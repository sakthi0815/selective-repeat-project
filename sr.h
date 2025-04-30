#ifndef SR_H
#define SR_H

#define PAYLOAD_SIZE 20
#define WINDOW_SIZE 8
#define TIMEOUT 16.0

struct msg {
    char data[PAYLOAD_SIZE];
};

struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[PAYLOAD_SIZE];
};

// Function prototypes
void A_output(struct msg message);
void A_input(struct pkt packet);
void A_timerinterrupt(void);
void A_init(void);
void B_input(struct pkt packet);
void B_init(void);
int compute_checksum(struct pkt packet);
int is_corrupt(struct pkt packet);

#endif




