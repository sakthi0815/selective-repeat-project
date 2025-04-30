#include <stdio.h>
#include <string.h>
#include "sr.h"
#include "emulator.h"

// Example window buffer
struct pkt window[WINDOW_SIZE];
int base = 0;
int nextseqnum = 0;

int compute_checksum(struct pkt packet) {
    int checksum = packet.seqnum + packet.acknum;
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        checksum += packet.payload[i];
    }
    return checksum;
}

int is_corrupt(struct pkt packet) {
    return packet.checksum != compute_checksum(packet);
}

void A_output(struct msg message) {
    if (nextseqnum < base + WINDOW_SIZE) {
        struct pkt packet;
        packet.seqnum = nextseqnum;
        packet.acknum = 0;
        memcpy(packet.payload, message.data, PAYLOAD_SIZE);
        packet.checksum = compute_checksum(packet);
        window[nextseqnum % WINDOW_SIZE] = packet;

        tolayer3(A, packet);
        if (base == nextseqnum) {
            starttimer(A, TIMEOUT);
        }
        nextseqnum++;
    }
}

void A_input(struct pkt packet) {
    if (!is_corrupt(packet)) {
        // Expected ACK
        int ack = packet.acknum;
        if (ack >= base && ack < nextseqnum) {
            // Slide window
            base = ack + 1;
            if (base == nextseqnum) {
                stoptimer(A);
            } else {
                starttimer(A, TIMEOUT);
            }
        }
    }
}

void A_timerinterrupt(void) {
    for (int i = base; i < nextseqnum; i++) {
        tolayer3(A, window[i % WINDOW_SIZE]);
    }
    starttimer(A, TIMEOUT);
}

void A_init(void) {
    base = 0;
    nextseqnum = 0;
}

void B_input(struct pkt packet) {
    if (!is_corrupt(packet)) {
        struct pkt ack_pkt;
        ack_pkt.seqnum = 0;
        ack_pkt.acknum = packet.seqnum;
        ack_pkt.checksum = compute_checksum(ack_pkt);
        tolayer3(B, ack_pkt);

        tolayer5(B, packet.payload);
    }
}

void B_init(void) {
    // Initialize receiver state if needed
}



