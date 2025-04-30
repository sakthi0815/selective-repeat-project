#ifndef SR_H
#define SR_H

#include "emulator.h"  // Include emulator for struct msg and pkt

#define A 0
#define B 1
#define PAYLOAD_SIZE 20
#define WINDOW_SIZE 8
#define TIMEOUT 20.0

// A-side
void A_output(struct msg message);
void A_input(struct pkt packet);
void A_timerinterrupt(void);
void A_init(void);

// B-side
void B_input(struct pkt packet);
void B_init(void);

// Helpers
int compute_checksum(struct pkt packet);
int is_corrupt(struct pkt packet);

#endif
