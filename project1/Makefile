CFLAGS = -fPIC -g -Wall 

test:libmalloc.a test.c
	gcc $(CFLAGS) -o test test.c libmalloc.a
	./test

malloc:libmalloc.a libmalloc.so
	echo done

libmalloc.a:my_malloc.o node.o
	ar r libmalloc.a my_malloc.o node.o
	
my_malloc.o:my_malloc.c my_malloc.h
	gcc $(CFLAGS) -c -o my_malloc.o my_malloc.c 
    
node.o:node.c node.h
	gcc $(CFLAGS) -c -o node.o node.c 
	
libmalloc.so:my_malloc.o node.o
	gcc $(CFLAGS) -shared -o libmalloc.so my_malloc.o node.o
	
	
	
intel-all:lib/libmalloc.so lib64/libmalloc.so
	echo intel-all
	
lib/libmalloc.so:lib malloc32.o node32.o
	gcc $(CFLAGS) -m32 -shared -o $@ malloc32.o node32.o
	
lib64/libmalloc.so:lib64 malloc64.o node64.o
	gcc $(CFLAGS) -shared -o $@ malloc64.o node64.o
	
lib:
	mkdir lib
	
lib64:
	mkdir lib64
	
malloc32.o:my_malloc.c
	gcc $(CFLAGS) -m32 -c my_malloc.c -o malloc32.o 
	
malloc64.o:my_malloc.c
	gcc $(CFLAGS) -c my_malloc.c -o malloc64.o
	
node32.o:node.c
	gcc $(CFLAGS) -m32 -c node.c -o node32.o
	
node64.o:node.c
	gcc $(CFLAGS) -c node.c -o node64.o
	

	

