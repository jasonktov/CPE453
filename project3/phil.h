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
