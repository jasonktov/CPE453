#include "lwp.h"

///////////////////////////////////////////////////////////////////////////////
//SCHEDULER
///////////////////////////////////////////////////////////////////////////////
Node *rr_head = NULL;
Node *run_node = NULL;

void rr_admit(thread new){
    if(thread == NULL){
        return;
    }
    
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node == NULL){
        printf("malloc failed");
        exit(1);
    }
    new_node->data = new;
    
    if(rr_head == NULL){
        Node *dummy_node = (Node*)malloc(sizeof(Node));
        rr_head = dummy_node;
        rr_head->next = new_node;
        new_node->prev = rr_head;
        
        run_node = new_node;
    }else{
        Node *cur_node = rr_head;
        while(cur_node->next != NULL){}
        cur_node->next = new_node;
        new_node->prev = cur_node;
    }
}

void rr_remove(thread victim){
    if(thread == NULL || rr_head == NULL){
        return;
    }
    
    Node *cur_node = rr_head;
    while(cur_node != NULL){
        if(cur_node->data == victim){
            Node *temp = cur_node->prev;
            cur_node->prev->next = cur_node->next;
            cur_node->next->prev = temp;
            free(cur_node->data);
            break;
        }else{
            cur_node = cur_node->next;
        }
    }
}

thread rr_next(void){
    if(rr_head == NULL || run_node == NULL){
        return NULL;
    }else if(run_node->next == NULL){
        return rr_head->next->data;
    }else{
        return run_node->next->data;
    }
}

int rr_qlen(void){
    Node *cur_node = rr_head;
    int count = 0;
    while(cur_node != NULL){
        count++;
        cur_node = cur_node->next;
    }
    return count;
}

struct scheduler rr_publish = {
    NULL, NULL, rr_admit, rr_remove, rr_next, rr_qlen
};
///////////////////////////////////////////////////////////////////////////////