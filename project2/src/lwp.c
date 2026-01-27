#include "../include/lwp.h"

///////////////////////////////////////////////////////////////////////////////
//SCHEDULER
///////////////////////////////////////////////////////////////////////////////
thread rr_head = NULL;//head of doubly linked list
thread rr_cur_thread = NULL;//keep track of current thread to find next()

static void rr_admit(thread new){
    //add thread to queue
    if(new == NULL){
        return;
    }
    
    if(rr_head == NULL){
        thread dummy = (thread)malloc(sizeof(context));
        rr_head = dummy;
        rr_head->sched_one = new;
        new->sched_two = rr_head;
        
        rr_cur_thread = new;
    }else{
        thread cur_thread = rr_head;
        while(cur_thread->sched_one != NULL){
            cur_thread = cur_thread->sched_one;
        }
        cur_thread->sched_one = new;
        new->sched_two = cur_thread;
    }
}

static void rr_remove(thread victim){
    //remove victim thread from queue
    if(rr_head == NULL || victim == NULL){
        printf("rr_remove():no threads initialized\n\r");
        return;
    }
    
    thread cur_thread = rr_head;
    while(cur_thread != NULL){
        if(cur_thread == victim){
            thread next = cur_thread->sched_one;
            thread prev = cur_thread->sched_two;
            
            cur_thread->sched_two->sched_one = next;
            if(cur_thread->sched_one != NULL){
                cur_thread->sched_one->sched_two = prev;
            }
            return;
        }else{
            cur_thread = cur_thread->sched_one;
        }
    }
    printf("rr_remove():t%ld not found\n\r", victim->tid);
    exit(1);
}

static thread rr_next(void){
    //return next thread in queue
    if(rr_head == NULL || rr_cur_thread == NULL){
        return NULL;
    }else if(rr_cur_thread->sched_one == NULL){
        rr_cur_thread = rr_head->sched_one;
    }else{
        rr_cur_thread = rr_cur_thread->sched_one;
    }
    return rr_cur_thread;
}

static int rr_qlen(void){
    //return length of queue
    thread cur_thread = rr_head;
    int count = 0;
    while(cur_thread != NULL){
        count++;
        cur_thread = cur_thread->lib_one;
    }
    return count;
}

struct scheduler rr_publish = {
    NULL, NULL, rr_admit, rr_remove, rr_next, rr_qlen
};
scheduler RoundRobin = &rr_publish;

///////////////////////////////////////////////////////////////////////////////
//LWP
///////////////////////////////////////////////////////////////////////////////
scheduler Sched = &rr_publish;
unsigned long ThreadIdCounter = 1;
size_t StackSize = 0;

//library's doubly linked list
context HeadContext;
thread HeadThread = &HeadContext;

//library's queue of waiting threads
context WaitContext;
thread WaitHeadThread = &WaitContext;

//currently running thread
thread ActiveThread = NULL;

static void lwp_wrap(lwpfun fun, void *arg){
    //function wrapper to manage return values & call exit
    int rval;
    rval = fun(arg);
    lwp_exit(rval);
}

static void calc_stack_size(void){
    //calculates stack size and assigns to global var StackSize
    long page_size = sysconf(_SC_PAGE_SIZE);
    if(page_size == -1){
        printf("stack_size(): Page size read error\n\r");
        return;
    }
    
    struct rlimit rlim;
    int stacksize_limit = DEFAULT_STACKSIZE;
    if(getrlimit(RLIMIT_STACK, &rlim) == -1){
        printf("stack_size(): limit read error, continuing with default\n\r");
    }
    if(rlim.rlim_cur != RLIM_INFINITY){
        stacksize_limit = rlim.rlim_cur;
    }
    
    int num_pages = (int)((float)stacksize_limit/(float)page_size);
    StackSize = (size_t)page_size * (size_t)num_pages;
}

tid_t lwp_create(lwpfun fun, void* arg){
    //Create thread and admit it to scheduler
    
    //calculate stack size
    if(StackSize == 0){
        //First thread created, calculate system's stack size
        calc_stack_size();
    }
    if(StackSize == -1){
        //calculation errored
        return NO_THREAD;
    }
    
    //initialize new stack
    unsigned long *newstack = mmap(NULL,
                    StackSize,
                    PROT_READ|PROT_WRITE,
                    MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK,
                    -1,0);
    if(newstack == NULL){
        printf("lwp_create(): mmap failed");
        return NO_THREAD;
    }
    
    //malloc to place context in heap
    thread newthread = (thread)malloc(sizeof(context));
    if(newthread == NULL){
        printf("lwp_create(): malloc failed\n\r");
        return NO_THREAD;
    }
    
    //initialize new thread
    newthread->tid = ThreadIdCounter;
    ThreadIdCounter++;
    newthread->stack = newstack;
    newthread->status = LWP_LIVE;
    
    //find end of allocated space which is start of stack
    unsigned long* stackstart = newstack+(StackSize/sizeof(unsigned long))-1;
    
    //Set up register context
    newthread->state.rax = 0;
    newthread->state.rbx = 0;
    newthread->state.rcx = 0;
    newthread->state.rdx = 0;
    newthread->state.rsi = (unsigned long)arg;
    newthread->state.rdi = (unsigned long)fun;
    newthread->state.rbp = (unsigned long)(stackstart - 2);
    newthread->state.r8  = 0;
    newthread->state.r9  = 0;
    newthread->state.r10 = 0;
    newthread->state.r11 = 0;
    newthread->state.r12 = 0;
    newthread->state.r13 = 0;
    newthread->state.r14 = 0;
    newthread->state.r15 = 0;
    newthread->state.fxsave = FPU_INIT;
    
    //Push pointers onto stack
    //address for swap_rfiles to return to
    *(stackstart) = (unsigned long)stackstart;
    
    //oldest base pointer, ends as base pointer
    *(stackstart - 1) = (unsigned long)(lwp_wrap);
    
    //pointer to old base pointer
    *(stackstart - 2) = (unsigned long)(stackstart - 1);
    
    //Update doubly linked list
    thread cur_thread = HeadThread;
    while(cur_thread->lib_one != NULL){
        cur_thread = cur_thread->lib_one;
    }
    cur_thread->lib_one = newthread;
    newthread->lib_two = cur_thread;
    
    
    //Admit to scheduler
    Sched->admit(newthread);
    return newthread->tid;
}

void lwp_start(void){
    //Converts running main thread into a lwp thread 
    // & begins running threads by calling lwp_yield()
    
    //malloc to place context in heap
    thread newthread = (thread)malloc(sizeof(context));
    if(newthread == NULL){
        printf("lwp_create(): malloc failed\n\r");
        return;
    }
    
    //initialize new thread
    newthread->tid = ThreadIdCounter;
    ThreadIdCounter++;
    unsigned long sp = 0;
    newthread->stack = &sp;
    newthread->status = LWP_LIVE;
    
    //Update list
    thread cur_thread = HeadThread;
    while(cur_thread->lib_one != NULL){
        cur_thread = cur_thread->lib_one;
    }
    cur_thread->lib_one = newthread;
    newthread->lib_two = cur_thread;
    
    //Admit to scheduler
    Sched->admit(newthread);
    
    //Set as active thread
    ActiveThread = newthread;
    
    //Begin by calling lwp_yield()
    lwp_yield();
}

void lwp_exit(int exitval){
    //terminates a thread by setting its status to terminated+returnval
    // terminated thread is removed from scheduler
    
    //set special terminated status
    ActiveThread->status = MKTERMSTAT(LWP_TERM, exitval);
    
    //remove from sched
    Sched->remove(ActiveThread);
    
    //readmit any waiting processes
    if(WaitHeadThread->exited != NULL){
        Sched->admit(WaitHeadThread->exited);
        WaitHeadThread->exited = WaitHeadThread->exited->exited;
    }
    
    lwp_yield();
}

void lwp_yield(void){
    //halts current thread and switches to another thread
    
    if(ActiveThread == NULL){
        printf("lwp_yield():threads not initialized\n\r");
        exit(1);
    }
    
    thread nextthread = Sched->next();
    if(nextthread == NULL){
        exit(1);
    }
    
    //save context
    swap_rfiles(&(ActiveThread->state), NULL);
    
    ActiveThread = nextthread;
    //restore context
    swap_rfiles(NULL, &(nextthread->state));
}

tid_t lwp_wait(int *status){
    //Deallocates any terminated threads. If none, current thread is removed
    //from scheduler and blocks until another thread calls lwp_exit()
    
    //look for terminated threads
    thread cur_thread;
    while(Sched->qlen() >= 1){
        cur_thread = HeadThread;
        while(cur_thread->lib_one != NULL){
            if(LWPTERMINATED(cur_thread->status) == FALSE){
                //keep iterating
                cur_thread = cur_thread->lib_one;
            }else{
                //found terminated thread
                tid_t tid = cur_thread->tid;
                if(status != NULL){
                    //returns return val
                    *status = (int)LWPTERMSTAT(cur_thread->status);
                }
                //remove from lib list
                thread next = cur_thread->lib_one;
                thread prev = cur_thread->lib_two;
                
                cur_thread->sched_two->lib_one = next;
                cur_thread->sched_one->lib_two = prev;
                
                //deallocate stack
                if(munmap(cur_thread->stack, StackSize) == -1){
                    printf("lwp_wait():munmap failed\n\r");
                }
                //deallocate thread context
                free(cur_thread);
                return tid;
            }
        }
        //no terminated threads, so block
        cur_thread = WaitHeadThread;
        while(WaitHeadThread->exited != NULL){
            WaitHeadThread = WaitHeadThread->exited;
        }
        
        //add to waiting queue
        WaitHeadThread->exited = ActiveThread;
        
        //remove from scheduler
        Sched->remove(ActiveThread);
        lwp_yield();
    }
    //No other ready threads
    return NO_THREAD;
}

thread tid2thread(tid_t tid){
    if(HeadThread == NULL){
        return NO_THREAD;
    }
    
    thread cur_thread = HeadThread;
    while(cur_thread != NULL){
        if(cur_thread->tid == tid){
            return cur_thread;
        }
    }
    return NO_THREAD;
}

tid_t lwp_gettid(void){
    return ActiveThread->tid;
}

void lwp_set_scheduler(scheduler s){
    if(Sched->init != NULL){
        Sched->init();
    }
    
    thread cur_thread = Sched->next();
    while(Sched->next() != NULL){
        Sched->remove(cur_thread);
        s->admit(cur_thread);
    }
    
    if(Sched->shutdown != NULL){
        Sched->shutdown();
    }
    Sched = s;
}

scheduler lwp_get_scheduler(void){
    return Sched;
}


