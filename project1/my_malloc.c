#include "my_malloc.h"

size_t heap_size = 0; //
size_t heap_length = 0;
void *heap_start = NULL;

///////////////////////////////////////////////////////////////////////////////
int checkHeapSize(size_t size){
    if(heap_length + sizeof(Node) + size > heap_size){ 
        //move break
        size_t amount_to_brk = BRK_SIZE * (1 + size/BRK_SIZE);
        void *sbrk_ret = sbrk((intptr_t)amount_to_brk); /*move data break*/
        if(sbrk_ret == (void *)-1){
            //no more space
            errno = ENOMEM;
            
            char buff[255];
            int len;
            len = snprintf(buff, sizeof(buff),
                    "MALLOC: sbrk returned -1\n");
            write(STDOUT_FILENO, buff, len);
            return -1;
        }
        if(heap_start == NULL){/*if this is first sbrk, record address*/
            heap_start = sbrk_ret;
        }
        
        heap_size += amount_to_brk;
        return amount_to_brk;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
void *my_malloc(size_t size){
    if(size == 0){
        return NULL;
    }
    
    size += MEM_ALIGN_SIZE - size%MEM_ALIGN_SIZE; /*resize for memory alignment*/
    
    if(checkHeapSize(size) == -1){
        //no more space
        return NULL;
    }
    Node *new_node = nodeAdd(heap_start, size);
    heap_length += nodeGetSize(new_node);
    return (void*)nodeGetStart(new_node);
}

void *malloc(size_t size){
    char buff[PBUFF_SIZE] = {0};
        int len;
        len = snprintf(buff, sizeof(buff), "MALLOC: sizeof(Node)=%zu\n", 
                        sizeof(Node));
        write(STDOUT_FILENO, buff, len);
    
    void* r = my_malloc(size);
    if(getenv("DEBUG_MALLOC")){
        char buff[PBUFF_SIZE] = {0};
        int len;
        if(r == NULL){
            len = snprintf(buff, sizeof(buff),
                    "MALLOC: malloc(%zu) => (ptr=%p, size=%d)\n", 
                    size, r, 0);
        }else{
            len = snprintf(buff, sizeof(buff),
                    "MALLOC: malloc(%zu) => (ptr=%p, size=%zu)\n", 
                    size, r, ((Node*)r - 1)->size_alloc);
                    /*pointer math to get to the header struct"*/
        }
        write(STDOUT_FILENO, buff, len);
    }
    return r;
}

///////////////////////////////////////////////////////////////////////////////
void *my_calloc(size_t n, size_t size){
    size_t s = n * size;
    if(n != 0 && s / n != size){
        char buff[255];
        int len;
        len = snprintf(buff, sizeof(buff),
                "MALLOC: calloc() integer overflow\n");
        write(STDOUT_FILENO, buff, len);
        exit(1);
    }
    
    void *mem = my_malloc(s);
    memset(mem, 0, s);
    return mem;
}

void *calloc(size_t n, size_t size){
    void* r = my_calloc(n, size);
    if(getenv("DEBUG_MALLOC")){
        char buff[PBUFF_SIZE] = {0};
        int len;
        if(r == NULL){
            len = snprintf(buff, sizeof(buff),
                    "MALLOC: calloc(%zu, %zu) => (ptr=%p, size=%d)\n", 
                    n, size, r, 0);
        }else{
            len = snprintf(buff, sizeof(buff),
                    "MALLOC: calloc(%zu, %zu) => (ptr=%p, size=%zu)\n", 
                    n, size, r, ((Node*)r - 1)->size_alloc);
                    /*pointer math to get to the header struct"*/
        }
        write(STDOUT_FILENO, buff, len);
    }
    return r;
}

///////////////////////////////////////////////////////////////////////////////
void my_free(void *p){
    if(p == NULL){
        return;
    }
    if(heap_start == NULL){
        return;
    }
    Node *cur_node = nodeFind(heap_start, (intptr_t)p);
    if(cur_node == NULL || cur_node == heap_start){
        char buff[255];
        int len;
        len = snprintf(buff, sizeof(buff),
                "MALLOC: free(%p) mem not found\n",
                p);
        write(STDOUT_FILENO, buff, len);
        return;
    }else{
        Node *prev = cur_node->prev;
        Node *next = cur_node->next;
        prev->next = next;
        if(next != NULL){
            //non-tail node
            next->prev = prev;
        }else{
            //tail node
            heap_length -= nodeGetSize(cur_node);
        }
    }
}

void free(void *p){
    if(p == NULL){
        return;
    }
    if(getenv("DEBUG_MALLOC")){
        char buff[PBUFF_SIZE] = {0};
        int len;
        len = snprintf(buff, sizeof(buff), "MALLOC: free(%p)\n", p);
        write(STDOUT_FILENO, buff, len);
    }
    my_free(p);
}

///////////////////////////////////////////////////////////////////////////////
void *my_realloc(void *p, size_t size){
    if(p == NULL){
        return my_malloc(size);
    }
    if(size == 0){
        my_free(p);
        return NULL;
    }
    if(heap_start == NULL){
        //no mem initialized yet
        char buff[255];
        int len;
        len = snprintf(buff, sizeof(buff),
                "MALLOC: realloc(%p, %zu) mem not found\n",
                p, size);
        write(STDOUT_FILENO, buff, len);
        return NULL;
    }
    
    Node *cur_node = (Node *)nodeFind(heap_start, (intptr_t)p);
    if(cur_node == NULL){
        //node not fouond
        char buff[255];
        int len;
        len = snprintf(buff, sizeof(buff),
                "MALLOC: realloc(%p, %zu) mem not found\n",
                p, size);
        write(STDOUT_FILENO, buff, len);
        return NULL;
    }
    
    size += MEM_ALIGN_SIZE - size%MEM_ALIGN_SIZE;/*resize for memory assignment*/
    if(cur_node->next == NULL){
        /*tail node*/
        if(size > cur_node->size_alloc){
            //expand
            checkHeapSize(size-cur_node->size_alloc);
            heap_length += size - cur_node->size_alloc;
        }else{
            //shrink
            heap_length -= cur_node->size_alloc - size;
        }
        cur_node->size_alloc = size;
        return (void *)nodeGetStart(cur_node);
    }else{
        /*non-tail node*/
        if(size > cur_node->size_alloc){
            /*expand*/
            size_t diff = size - cur_node->size_alloc;
            if(nodeGetGap(cur_node) > diff){
                //in place
                cur_node->size_alloc = size;
                return (void *)nodeGetStart(cur_node);
            }else{
                //copy
                void *dest = my_malloc(size);
                if(dest == NULL){
                    return NULL;
                }else{
                    nodeCopy(cur_node, (Node *)dest - 1);
                    /*pointer math to point to header*/
                    my_free(cur_node);
                    return (void*)dest;
                }
            }
        }else{
            /*shrink*/
            cur_node->size_alloc = size;
            return (void *)nodeGetStart(cur_node);
        }
    }
    return NULL;
}

void *realloc(void *p, size_t size){
    void* r = my_realloc(p, size);
    if(getenv("DEBUG_MALLOC")){
        char buff[PBUFF_SIZE] = {0};
        int len;
        if(r == NULL){
            len = snprintf(buff, sizeof(buff),
                            "MALLOC: realloc(%p, %zu) => (ptr=%p, size=%d)\n", 
                            p, size, r, 0);
        }else{
            len = snprintf(buff, sizeof(buff),
                            "MALLOC: realloc(%p, %zu) => (ptr=%p, size=%zu)\n",
                            p, size, r, ((Node*)r - 1)->size_alloc);
                            /*pointer math to get to the header struct"*/
        }
        write(STDOUT_FILENO, buff, len);
    }
    return r;
}