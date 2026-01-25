#include "lwp.h"

///////////////////////////////////////////////////////////////////////////////
//SCHEDULER
///////////////////////////////////////////////////////////////////////////////
Node *rr_head = NULL;
Node *run_node = NULL;

void rr_admit(thread new){
    if(new == NULL){
        return;
    }
    
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node == NULL){
        printf("rr_admit(): malloc failed\n\r");
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
    if(victim == NULL || victim == NULL){
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
//LWP
///////////////////////////////////////////////////////////////////////////////
tid_t lwp_create(lwpfun fun, void* arg){
    long page_size = sysconf(_SC_PAGE_SIZE);
    if(page_size == -1){
        printf("lwp_create(): Page size read error\n\r");
        return NULL;
    }
    
    struct rlimit rlim;
    int stacksize_limit = DEFAULT_STACKSIZE;
    if(getrlimit(RLIMIT_STACK, *rlim) == -1){
        printf("lwp_create(): Stack size limit read error, continuing with default\n\r");
    }
    if(rlim.rlim_cur != RLIM_INFINITY){
        stacksize_limit = rlim.rlim_cur;
    }
    
    size_t stacksize = (size_t)page_size * (size_t)((float)stacksize_limit/(float)page_size);
    
    thread t = mmap(NULL,
                    stacksize,
                    PROT READ|PROT WRITE,
                    MAP PRIVATE|MAP ANONYMOUS|MAP STACK,
                    -1,0);
}
