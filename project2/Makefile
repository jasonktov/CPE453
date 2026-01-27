CFLAGS = -fPIC -g -Wall 

liblwp.so:lwp.o magic64.o
	gcc $(CFLAGS) -shared -o liblwp.so lwp.o magic64.o

lwp.o:lwp.c lwp.h fp.h
	gcc $(CFLAGS) -o lwp.o -c lwp.c

magic64.o:magic64.S
	gcc $(CFLAGS) -o magic64.o -c magic64.S
	
test:liblwp.so test.c
	gcc $(CFLAGS) test.c -L. -llwp -o test 

	

