#ifndef SR_H
#define SR_H

#include "emulator.h"  // Include emulator.h, which includes emulator.h definitions

#define WINDOW_SIZE 8
#define TIMEOUT 30
#define PAYLOAD_SIZE 20

// Declare external variables
extern int nextseqnum;
extern int base;
extern struct pkt window[WINDOW_SIZE];

void A_output(struct msg message);
void A_input(struct pkt packet);
void A_timerinterrupt(void);
void B_input(struct pkt packet);
int compute_checksum(struct pkt packet);
int is_corrupt(struct pkt packet);

// Declare functions
extern void A_init(void);
extern void B_init(void);
extern void B_output(struct msg message);
extern void B_timerinterrupt(void);

#endif
