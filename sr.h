#ifndef SR_H
#define SR_H

#include "emulator.h"  // Ensure emulator.h is included once for msg and pkt structs

// Define constants for the window size and timeout
#define WINDOW_SIZE 4  // Set your desired window size here
#define TIMEOUT 16.0   // Timeout value for retransmissions

// Function declarations
void A_init();
void A_output(struct msg message);
void A_input(struct pkt packet);
void A_timerinterrupt();

void B_init();
void B_input(struct pkt packet);

extern struct pkt window[WINDOW_SIZE];  // Window array for Selective Repeat

// Function prototypes
int compute_checksum(struct pkt packet);
int is_corrupt(struct pkt packet);

#endif
