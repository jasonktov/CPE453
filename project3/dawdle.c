#include "dawdle.h"

void dawdle(){
    struct timespec tv;
    int msec = (int)((((double)random())/RAND_MAX)*DAWDLEFACTOR);
    
    tv.tv_sec = 0;
    tv.tv_nsec = 1000000 * msec;
    if(-1 == nanosleep(&tv, NULL)){
        perror("nanosleep");
    }
}