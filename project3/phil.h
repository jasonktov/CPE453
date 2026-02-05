#ifndef PHILH
#define PHILH

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include "dawdle.h"

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define EAT_STR "Eat  "
#define THINK_STR "Think"
#define CHANGE_STR "     "
#define STATE_STR_LEN 6

#define NO_FORK -1

typedef enum phil_state{
    PHIL_CHANGING, PHIL_EATING, PHIL_THINKING, PHIL_TERM
}p_state;

typedef struct phil_status{
    int left_fork;
    int right_fork;
    p_state state;
}p_status;

#endif
