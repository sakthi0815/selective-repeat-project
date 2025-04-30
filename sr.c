#include <stdio.h>
#include <string.h>
#include "sr.h"

int base = 0;
int nextseqnum = 0;
struct pkt window[WINDOW_SIZE];

int compute_checksum(struct pkt packet) {
    int sum = packet.seqnum + packet.acknum;
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        sum += packet.payload[i];
    }
    return sum;
}

int is_corrupt(struct pkt packet) {
    return compute_checksum(packet) != packet.checksum;
}

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

void A_timerinterrupt(void) {
    printf("A_timerinterrupt: resending all unACKed packets\n");
    for (int i = base; i < nextseqnum; i++) {
        printf("A_timerinterrupt: retransmitting seqnum = %d\n", i);
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

void B_init(void) {
    // Nothing to initialize for now
}

// Stub functions required by emulator but unused in unidirectional A->B transfer
void B_output(struct msg message) {
    // Not used in this assignment
}

void B_timerinterrupt(void) {
    // Not used in this assignment
}