#include "sr.h"  // This includes emulator.h via sr.h
#include <stdio.h>
#include <string.h>

// Global variables
int base = 0;
int nextseqnum = 0;
struct pkt window[WINDOW_SIZE];  // Window array for Selective Repeat

// Dummy function for B_output (receiver side output)
void B_output(struct msg message) {
    // Do nothing for SR, as B does not need to process messages from A
    return;
}

// Dummy function for B_timerinterrupt (receiver side timer interrupt)
void B_timerinterrupt() {
    // Do nothing for SR, as B does not use a timer
    return;
}

// Function to compute checksum for a packet
int compute_checksum(struct pkt packet) {
    int checksum = 0;
    for (int i = 0; i < sizeof(packet.payload); i++) {
        checksum += packet.payload[i];
    }
    checksum += packet.seqnum + packet.acknum;
    return checksum;
}

// Function to check if a packet is corrupt
int is_corrupt(struct pkt packet) {
    return compute_checksum(packet) != packet.checksum;
}

// Sender A-side: Initialize necessary variables
void A_init() {
    base = 0;
    nextseqnum = 0;
    memset(window, 0, sizeof(window));
}

// Sender A-side: Output function called when sending a message
void A_output(struct msg message) {
    if (nextseqnum < base + WINDOW_SIZE) {
        struct pkt packet;
        packet.seqnum = nextseqnum;
        packet.acknum = 0;
        memcpy(packet.payload, message.data, sizeof(message.data));

        // Compute checksum and add to packet
        packet.checksum = compute_checksum(packet);

        // Send packet to the network
        tolayer3(0, packet);

        // Start the timer if it's the first packet in the window
        if (nextseqnum == base) {
            starttimer(0, TIMEOUT);
        }

        // Add packet to the window
        window[nextseqnum % WINDOW_SIZE] = packet;

        // Increment sequence number
        nextseqnum++;
    }
}

// Sender A-side: Input function called when receiving an acknowledgment
void A_input(struct pkt packet) {
    if (is_corrupt(packet)) {
        printf("A_input: ACK corrupted, ignored\n");
        return;
    }

    // Update base if the ACK is valid
    if (packet.acknum >= base) {
        base = packet.acknum + 1;
    }

    // Stop the timer if all packets are acknowledged
    if (base == nextseqnum) {
        stoptimer(0);
    } else {
        // Restart the timer if needed
        starttimer(0, TIMEOUT);
    }
}

// Sender A-side: Timer interrupt function
void A_timerinterrupt() {
    printf("A_timerinterrupt: resending all unACKed packets\n");

    // Resend all unACKed packets in the window
    for (int i = base; i < nextseqnum; i++) {
        tolayer3(0, window[i % WINDOW_SIZE]);
    }

    // Restart the timer
    starttimer(0, TIMEOUT);
}

// Receiver B-side: Input function called when receiving a packet
void B_input(struct pkt packet) {
    if (is_corrupt(packet)) {
        printf("B_input: packet corrupted, ignoring\n");
        return;
    }

    // If the packet is in order, deliver it to layer 5
    if (packet.seqnum == base) {
        tolayer5(1, packet.payload);
        base++;

        // Send an acknowledgment for the packet
        struct pkt ack_pkt;
        ack_pkt.seqnum = 0;
        ack_pkt.acknum = base;
        ack_pkt.checksum = compute_checksum(ack_pkt);
        tolayer3(1, ack_pkt);
    } else {
        // If the packet is out of order, just send an acknowledgment
        struct pkt ack_pkt;
        ack_pkt.seqnum = 0;
        ack_pkt.acknum = base;
        ack_pkt.checksum = compute_checksum(ack_pkt);
        tolayer3(1, ack_pkt);
    }
}

// Receiver B-side: Initialize necessary variables
void B_init() {
    base = 0;
}
