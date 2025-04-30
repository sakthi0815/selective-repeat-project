#include "sr.h"
#include <stdio.h>
#include <string.h>

// Initialize global variables
int nextseqnum = 0;
int base = 0;
struct pkt window[WINDOW_SIZE];

// Initialize A
void A_init(void) {
    nextseqnum = 0;
    base = 0;
    printf("A_init: sender initialized\n");
}

// Initialize B
void B_init(void) {
    printf("B_init: receiver initialized\n");
}

// Function to output from A
void A_output(struct msg message) {
    if (nextseqnum < base + WINDOW_SIZE) {
        struct pkt packet;
        packet.seqnum = nextseqnum;
        packet.acknum = 0;
        memcpy(packet.payload, message.data, PAYLOAD_SIZE);
        packet.checksum = compute_checksum(packet);

        window[nextseqnum % WINDOW_SIZE] = packet;

        printf("A_output: sending packet with seqnum = %d\n", packet.seqnum);
        tolayer3(A, packet);

        if (base == nextseqnum) {
            starttimer(A, TIMEOUT);
        }

        nextseqnum++;
    } else {
        printf("A_output: window full, dropping message\n");
    }
}

// Function to input to A
void A_input(struct pkt packet) {
    if (!is_corrupt(packet)) {
        printf("A_input: received ACK for seqnum = %d\n", packet.acknum);

        if (packet.acknum >= base && packet.acknum < nextseqnum) {
            base = packet.acknum + 1;

            if (base == nextseqnum) {
                stoptimer(A);
            } else {
                starttimer(A, TIMEOUT);
            }
        }
    } else {
        printf("A_input: ACK corrupted, ignored\n");
    }
}

// Function to handle timer interrupt in A
void A_timerinterrupt(void) {
    printf("A_timerinterrupt: resending all unACKed packets\n");

    for (int i = base; i < nextseqnum; i++) {
        tolayer3(A, window[i % WINDOW_SIZE]);
    }

    starttimer(A, TIMEOUT);
}

// Function to input to B
void B_input(struct pkt packet) {
    if (!is_corrupt(packet)) {
        printf("B_input: received expected packet seqnum = %d\n", packet.seqnum);

        struct pkt ack_pkt;
        ack_pkt.seqnum = 0;
        ack_pkt.acknum = packet.seqnum;
        ack_pkt.checksum = compute_checksum(ack_pkt);
        tolayer3(B, ack_pkt);

        tolayer5(B, packet.payload);
    } else {
        printf("B_input: packet corrupted, ignoring\n");
    }
}

// Function to compute checksum
int compute_checksum(struct pkt packet) {
    int checksum = 0;
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        checksum += packet.payload[i];
    }
    checksum += packet.seqnum + packet.acknum;
    return checksum;
}

// Function to check if packet is corrupted
int is_corrupt(struct pkt packet) {
    return compute_checksum(packet) != packet.checksum;
}

// Placeholder function for B_output
void B_output(struct msg message) {
    // Placeholder function for B_output
    printf("B_output: this is a placeholder\n");
}

// Placeholder function for B_timerinterrupt
void B_timerinterrupt(void) {
    // Placeholder function for B_timerinterrupt
    printf("B_timerinterrupt: this is a placeholder\n");
}
