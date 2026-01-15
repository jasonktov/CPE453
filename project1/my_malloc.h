

#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>


#define BRK_SIZE 0xFFFF
#define MEM_ALIGN_SIZE 16
#define PBUFF_SIZE 255

#include "node.h"

void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void free(void *p);
void *realloc(void *p, size_t size);

#endif