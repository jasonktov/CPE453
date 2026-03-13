CFLAGS = -fPIC -g -Wall -std=c99

all:minls minget

minget: minget.o minprint.o
	gcc $(CFLAGS) -o minget minget.o minprint.o

minls: minls.o minprint.o
	gcc $(CFLAGS) -o minls minls.o minprint.o

minget.o: minget.c minls.h minprint.h
	gcc $(CFLAGS) -o minget.o -c minget.c
	
minls.o: minls.c minls.h minprint.h
	gcc $(CFLAGS) -o minls.o -c minls.c
	
minprint.o: minprint.c minls.h minprint.h
	gcc $(CFLAGS) -o minprint.o -c minprint.c