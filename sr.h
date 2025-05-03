#ifndef _SR_H_
#define _SR_H_

/* Define structures to match emulator.h */
struct msg {
    char data[20];
};

struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

/* Event structure for compatibility with emulator */
typedef struct {
    float time;
    int type;
    int entity;
    struct pkt packet;
    struct msg message;
} event;

#define WINDOW_SIZE 6
#define TIMEOUT 40.0  // Increased to 40.0 to improve delivery
#define MAX_SEQ 31

/* Function prototypes using struct pkt and struct msg */
extern float get_clocktime(void);
extern void tolayer3(int entity, struct pkt packet);
extern void tolayer5(int entity, char *data);
extern void starttimer(int entity, float increment);
extern void stoptimer(int entity);
extern void generate_next_arrival(void);
extern void init(void);
extern int getwinsize(void);

extern int TRACE;
extern int nsim;
extern int nsimmax;
extern float lossprob;
extern float corruptprob;
extern float lambda;
extern int ntolayer3;
extern int nlost;
extern int ncorrupt;
extern int window_full;
extern int packets_resent;
extern int total_ACKs_received;
extern int new_ACKs;
extern int packets_received;

#endif
