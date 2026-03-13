CFLAGS = -fPIC -g -Wall -std=c99

all:minls

minls: minls.o minprint.o
	gcc $(CFLAGS) -o minls minls.o minprint.o
	
minls.o:minls.c minls.h minprint.h
	gcc $(CFLAGS) -o minls.o -c minls.c
	
minprint.o: minprint.c minls.h minprint.h
	gcc $(CFLAGS) -o minprint.o -c minprint.c