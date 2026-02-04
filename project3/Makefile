

dine:phil.o dawdle.o
	gcc --std=c99 -g -Wall -fPIC -o dine phil.o dawdle.o
	
phil.o:phil.c
	gcc --std=c99 -g -Wall -fPIC -o phil.o -c phil.c -lpthread
	
dawdle.o:dawdle.c
	gcc -g -Wall -o dawdle.o -c dawdle.c -lrt
