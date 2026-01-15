#include "node.h"

intptr_t nodeGetSize(Node *n){
    /*return total size of node*/
    return (intptr_t)sizeof(Node) + (intptr_t)n->size_alloc;
}

intptr_t nodeGetEnd(Node *n){
    /*return address of data end*/
    return (intptr_t)n+(intptr_t)sizeof(Node)+(intptr_t)n->size_alloc;
}

intptr_t nodeGetStart(Node *n){
    /*return start of data*/
    return (intptr_t)n + (intptr_t)sizeof(Node);
}

intptr_t nodeGetGap(Node *n){
    /*return gap between end of node's data and start of next node*/
    if(n == NULL){
        return -1;
    }else if(n->next == NULL){
        return -1;
    }else{
        return (intptr_t)n->next - nodeGetEnd(n);
    }
}

void nodeCopy(Node *s, Node *d){
    /*copy data from s to d*/
    uint8_t* s_start = (uint8_t*)nodeGetStart(s);
    uint8_t* d_start = (uint8_t*)nodeGetStart(d);
    int i;
    for(i = 0; i < s->size_alloc; i++){
        d_start[i] = s_start[i];
    }
}

Node *nodeAdd(void *heap_start, size_t size){
    Node *cur_node = (Node *)heap_start;
    uint8_t inbetween = 0;
    
    /*iterate through nodes to search for space, if yes, then set inbetween*/
    while(cur_node->next != NULL){ 
        if(nodeGetGap(cur_node) > (intptr_t)size){
            inbetween = 1;
            break;
        }
        cur_node = cur_node->next;
    }
    
    Node *next;
    if(inbetween){
        next = cur_node->next;
    }else{
        next = NULL;
    }
    
    
    cur_node->next = (Node *)(nodeGetEnd(cur_node)); /*assign address to next*/
    
    Node *new_node = cur_node->next; /*treat next address as new Node*/
    
    new_node->size_alloc = size;
    new_node->prev = cur_node;
    new_node->next = next; /*either NULL or next*/
   
    return new_node;
}

Node *nodeFind(void *heap_start, intptr_t ptr){
    Node *cur_node = heap_start;
    
    int searching = 1;
    while(searching){
        if((intptr_t)cur_node == ptr){
            searching = 0;
        }else if((ptr > (intptr_t)cur_node)&&(ptr < nodeGetEnd(cur_node))){
            searching = 0;
        }else if(cur_node->next != NULL){
            cur_node = cur_node->next;
        }else{
            return NULL;
        }
    }
    return cur_node;
}