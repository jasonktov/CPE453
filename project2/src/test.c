#include <stdio.h>
#include "../include/lwp.h"
///////////////////////////////////////////////////////////////////////////////
//Testing
///////////////////////////////////////////////////////////////////////////////
int test(void){
    printf("test():t%ld pre-yield\n\r", lwp_gettid());
    lwp_yield();
    printf("test():t%ld post-yield\n\r", lwp_gettid());
    return 3;
}

int main(void){
    tid_t t1 = lwp_create((lwpfun)test, NULL);
    tid_t t2 = lwp_create((lwpfun)test, NULL);
    tid_t t3 = lwp_create((lwpfun)test, NULL);
    printf("main():created t%ld t%ld t%ld\n\r", t1,t2,t3);
    
    lwp_start();
    lwp_exit(1);
    /*
    tid_t r1 = lwp_wait(NULL);
    tid_t r2 = lwp_wait(NULL);
    tid_t r3 = lwp_wait(NULL);
    printf("main():waited for t%ld t%ld t%ld\n\r", r1,r2,r3);
    */
    return 0;
}
