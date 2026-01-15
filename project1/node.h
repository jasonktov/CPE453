

#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#define MEM_ALIGN_SIZE 16

struct node{
    size_t size_alloc;
    struct node *prev;
    struct node *next;
}  __attribute__((aligned(MEM_ALIGN_SIZE)));
typedef struct node Node;

intptr_t nodeGetSize(Node* n);
intptr_t nodeGetEnd(Node* n);
intptr_t nodeGetStart(Node* n);
intptr_t nodeGetGap(Node* n);

void nodeCopy(Node* s, Node* d);

Node *nodeAdd(void* heap_start, size_t s);
Node *nodeFind(void *heap_start, intptr_t p);

#endif