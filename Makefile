all: sr

sr: sr.o emulator.o
	gcc -Wall -std=c99 -pedantic -o sr sr.o emulator.o

sr.o: sr.c sr.h
	gcc -Wall -std=c99 -pedantic -c sr.c

emulator.o: emulator.c emulator.h gbn.h
	gcc -Wall -std=c99 -pedantic -c emulator.c

clean:
	rm -f sr *.o
