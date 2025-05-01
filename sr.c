#include "sr.h"
#include "emulator.h"

// Global variables for sliding window protocol
int base = 0;
int nextseqnum = 0;
struct pkt window[WINDOW_SIZE];

void A_output(struct msg message) {
    if (nextseqnum < base + WINDOW_SIZE) {
        struct pkt packet;
        packet.seqnum = nextseqnum;
        memcpy(packet.payload, message.data, PAYLOAD_SIZE);
        packet.checksum = compute_checksum(packet);

        printf("A_output: sending packet with seqnum = %d\n", packet.seqnum);
        tolayer3(A, packet);
        starttimer(A, TIMEOUT);
        window[nextseqnum % WINDOW_SIZE] = packet;
        nextseqnum++;
    } else {
        printf("A_output: window full, dropping message\n");
    }
}

void A_input(struct pkt packet) {
    if (is_corrupt(packet)) {
        printf("A_input: ACK corrupted, ignored\n");
        return;
    }

    if (packet.acknum >= base && packet.acknum < nextseqnum) {
        base = packet.acknum + 1;
        printf("A_input: received ACK for seqnum = %d\n", packet.acknum);
        stoptimer(A);
        if (base < nextseqnum) {
            starttimer(A, TIMEOUT);
        }
    }
}

void A_timerinterrupt() {
    printf("A_timerinterrupt: resending all unACKed packets\n");
    for (int i = base; i < nextseqnum; i++) {
        tolayer3(A, window[i % WINDOW_SIZE]);
    }
    starttimer(A, TIMEOUT);
}
