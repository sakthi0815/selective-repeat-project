CC = gcc
CFLAGS = -Wall -std=c99 -pedantic

SR_SRCS = sr.c emulator.c
SR_HDRS = sr.h emulator.h

all: sr

sr: $(SR_SRCS) $(SR_HDRS)
	$(CC) $(CFLAGS) -o sr $(SR_SRCS)

clean:
	rm -f sr *.o
