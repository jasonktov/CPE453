CFLAGS = -fPIC -g -Wall 

lib64/liblwp.so:build/lwp.o build/magic64.o
	gcc $(CFLAGS) -shared -o lib64/liblwp.so build/lwp.o build/magic64.o

build/lwp.o:src/lwp.c include/lwp.h include/fp.h
	gcc $(CFLAGS) -o build/lwp.o -c src/lwp.c

build/magic64.o:src/magic64.S
	gcc $(CFLAGS) -o build/magic64.o -c src/magic64.S
	
test:lib64/liblwp.so src/test.c
	gcc $(CFLAGS) src/test.c -L./lib64 -llwp -o build/test 

	

