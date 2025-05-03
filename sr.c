#include <stdio.h>
#include <string.h>
#include "sr.h"

int base = 0;
int nextseqnum = 0;
int N; // Window size
struct pkt A_window[WINDOW_SIZE];
int A_ack[WINDOW_SIZE];
int timer_running = 0;  // Track timer state
int retransmit_count[WINDOW_SIZE];  // Track retransmissions per packet

int expectedseqnum = 0;
struct pkt B_buffer[WINDOW_SIZE];
int B_received[WINDOW_SIZE];

int compute_checksum(struct pkt *packet) {
    int checksum = packet->seqnum + packet->acknum;
    for (int i = 0; i < 20; i++) {
        checksum += packet->payload[i];
    }
    return checksum;
}

void A_output(struct msg message) {
    if (((nextseqnum - base + MAX_SEQ + 1) % (MAX_SEQ + 1)) < N) {
        struct pkt packet;
        packet.seqnum = nextseqnum;
        packet.acknum = 0;
        memcpy(packet.payload, message.data, 20);
        packet.checksum = compute_checksum(&packet);

        A_window[nextseqnum % N] = packet;
        A_ack[nextseqnum % N] = 0;
        retransmit_count[nextseqnum % N] = 0;  // Initialize retransmit count

        tolayer3(0, packet);
        if (TRACE > 0)
            printf("----A: Sending packet %d\n", packet.seqnum);

        if (!timer_running && base != nextseqnum) {
            starttimer(0, TIMEOUT);
            timer_running = 1;
            if (TRACE > 1)
                printf("          START TIMER: starting timer\n");
        }
        nextseqnum = (nextseqnum + 1) % (MAX_SEQ + 1);
    } else {
        if (TRACE > 0)
            printf("----A: Window full, message dropped\n");
        window_full++;
    }
}

void A_input(struct pkt packet) {
    int checksum = compute_checksum(&packet);
    if (checksum != packet.checksum) {
        if (TRACE > 0)
            printf("----A: Corrupted ACK received, ignoring\n");
        return;
    }

    if (packet.acknum < 0 || packet.acknum > MAX_SEQ) {
        if (TRACE > 0)
            printf("----A: Invalid ACK %d, ignoring\n", packet.acknum);
        return;
    }

    if (TRACE > 0)
        printf("----A: Received ACK %d, base=%d, nextseqnum=%d\n", packet.acknum, base, nextseqnum);

    total_ACKs_received++;

    // Mark packets as acknowledged up to packet.acknum within the window
    int marked = 0;
    int seq = base;
    int dist_to_ack = (packet.acknum - base + MAX_SEQ + 1) % (MAX_SEQ + 1);
    int dist_to_next = (nextseqnum - base + MAX_SEQ + 1) % (MAX_SEQ + 1);
    if (dist_to_ack <= dist_to_next) { // Ensure ACK is within or before the window
        while (((seq - base + MAX_SEQ + 1) % (MAX_SEQ + 1)) <= dist_to_ack) {
            int slot = seq % N;
            if (!A_ack[slot] && A_window[slot].seqnum == seq) {
                A_ack[slot] = 1;
                marked++;
                retransmit_count[slot] = 0;  // Reset retransmit count
                if (TRACE > 1)
                    printf("----A: Marked packet %d as acknowledged\n", seq);
            }
            seq = (seq + 1) % (MAX_SEQ + 1);
        }
    }

    if (marked > 0) {
        new_ACKs += marked;
    }

    // Slide the window to the next unacknowledged packet
    while (base != nextseqnum && A_ack[base % N]) {
        A_ack[base % N] = 0;
        retransmit_count[base % N] = 0;  // Reset retransmit count
        base = (base + 1) % (MAX_SEQ + 1);
        if (TRACE > 1)
            printf("----A: Slid window, new base=%d\n", base);
    }

    // Timer management: Stop if window is empty, start if unacknowledged packets exist
    if (base == nextseqnum) {
        if (timer_running) {
            stoptimer(0);
            timer_running = 0;
            if (TRACE > 1)
                printf("----A: Stopped timer, window empty\n");
        }
    } else if (!timer_running) {
        starttimer(0, TIMEOUT);
        timer_running = 1;
        if (TRACE > 1)
            printf("          START TIMER: starting timer\n");
    }
}

void A_timerinterrupt(void) {
    if (TRACE > 0)
        printf("----A: Timer interrupt, resending packets from base=%d to nextseqnum-1=%d\n", base, (nextseqnum - 1) % (MAX_SEQ + 1));

    if (base != nextseqnum) {
        // Stop any existing timer before retransmitting
        if (timer_running) {
            stoptimer(0);
            timer_running = 0;
            if (TRACE > 1)
                printf("          STOP TIMER: stopping timer\n");
        }

        int seq = base;
        int resent = 0;
        while (seq != nextseqnum) {
            int slot = seq % N;
            if (!A_ack[slot]) {
                tolayer3(0, A_window[slot]);
                packets_resent++;
                retransmit_count[slot]++;
                if (TRACE > 1)
                    printf("----A: Resent packet %d (retransmit count %d)\n", seq, retransmit_count[slot]);
                resent = 1;

                // Limit retransmissions to avoid infinite loops (e.g., 15 attempts per packet)
                if (retransmit_count[slot] > 15) {
                    if (TRACE > 0)
                        printf("----A: Max retransmits reached for packet %d, giving up\n", seq);
                    A_ack[slot] = 1;  // Mark as acknowledged to move forward
                }
            }
            seq = (seq + 1) % (MAX_SEQ + 1);
        }

        // Start a new timer only if packets were resent
        if (resent) {
            starttimer(0, TIMEOUT);
            timer_running = 1;
            if (TRACE > 1)
                printf("          START TIMER: starting timer\n");
        }
    } else {
        if (timer_running) {
            stoptimer(0);
            timer_running = 0;
            if (TRACE > 1)
                printf("----A: Stopped timer, no unacknowledged packets\n");
        }
    }
}

void A_init(void) {
    base = 0;
    nextseqnum = 0;
    N = getwinsize();
    timer_running = 0;
    for (int i = 0; i < N; i++) {
        A_ack[i] = 0;
        retransmit_count[i] = 0;
    }
}

void B_input(struct pkt packet) {
    int checksum = compute_checksum(&packet);
    if (checksum != packet.checksum) {
        if (TRACE > 0)
            printf("----B: Corrupted packet %d, sending ACK %d\n", packet.seqnum, (expectedseqnum - 1 + MAX_SEQ + 1) % (MAX_SEQ + 1));
        struct pkt ackpkt;
        ackpkt.seqnum = 0;
        ackpkt.acknum = (expectedseqnum - 1 + MAX_SEQ + 1) % (MAX_SEQ + 1);
        memset(ackpkt.payload, 0, 20);
        ackpkt.checksum = compute_checksum(&ackpkt);
        tolayer3(1, ackpkt);
        return;
    }

    int dist_to_seq = (packet.seqnum - expectedseqnum + MAX_SEQ + 1) % (MAX_SEQ + 1);
    if (TRACE > 1)
        printf("----B: Received packet %d, expectedseqnum=%d, dist_to_seq=%d, window_size=%d\n", packet.seqnum, expectedseqnum, dist_to_seq, N);

    if (dist_to_seq < N) {
        if (!B_received[packet.seqnum % N]) {
            B_buffer[packet.seqnum % N] = packet;
            B_received[packet.seqnum % N] = 1;
            if (TRACE > 1)
                printf("----B: Buffered packet %d\n", packet.seqnum);
        }

        // Deliver packets in sequence starting from expectedseqnum
        int seq = expectedseqnum;
        while (B_received[seq % N]) {
            struct msg message;
            memcpy(message.data, B_buffer[seq % N].payload, 20);
            tolayer5(1, message.data);
            packets_received++;
            if (TRACE > 0)
                printf("----B: Delivered packet %d to layer 5\n", seq);
            B_received[seq % N] = 0;
            seq = (seq + 1) % (MAX_SEQ + 1);
        }
        int highest_delivered = (seq - 1 + MAX_SEQ + 1) % (MAX_SEQ + 1);
        expectedseqnum = seq;

        // Send cumulative ACK for the highest delivered packet
        struct pkt ackpkt;
        ackpkt.seqnum = 0;
        ackpkt.acknum = highest_delivered;
        memset(ackpkt.payload, 0, 20);
        ackpkt.checksum = compute_checksum(&ackpkt);
        tolayer3(1, ackpkt);
        if (TRACE > 0)
            printf("----B: Sent ACK %d\n", ackpkt.acknum);
    } else {
        if (TRACE > 0)
            printf("----B: Packet %d outside window, sending ACK %d\n", packet.seqnum, (expectedseqnum - 1 + MAX_SEQ + 1) % (MAX_SEQ + 1));
        struct pkt ackpkt;
        ackpkt.seqnum = 0;
        ackpkt.acknum = (expectedseqnum - 1 + MAX_SEQ + 1) % (MAX_SEQ + 1);
        memset(ackpkt.payload, 0, 20);
        ackpkt.checksum = compute_checksum(&ackpkt);
        tolayer3(1, ackpkt);
    }
}

void B_init(void) {
    expectedseqnum = 0;
    N = getwinsize();
    for (int i = 0; i < N; i++) {
        B_received[i] = 0;
    }
}

void B_output(struct msg message) {
    // Not used
}

void B_timerinterrupt(void) {
    // Not used
}

int getwinsize(void) {
    return WINDOW_SIZE;
}
