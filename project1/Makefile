CFLAGS = -fPIC -g -Wall 

malloc:libmalloc.a libmalloc.so
	echo done

libmalloc.a:my_malloc.o node.o
	ar r libmalloc.a my_malloc.o node.o
	
my_malloc.o:my_malloc.c my_malloc.h
	gcc $(CFLAGS) -c my_malloc.o my_malloc.c 
    
node.o:node.c node.h
	gcc $(CFLAGS) -c node.o node.c 
	
libmalloc.so:my_malloc.o node.o
	gcc $(CFLAGS) -shared -o libmalloc.so my_malloc.o node.o
	

