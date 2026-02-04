#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_PHILOSOPHERS 5
#define NUM_MAX_EATERS 2

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef enum phil_state{
    PHIL_CHANGING, PHIL_EATING, PHIL_THINKING, PHIL_TERM
}p_state;

int eaters = NUM_MAX_EATERS;

sem_t forks[NUM_PHILOSOPHERS];
sem_t eater;
sem_t printer;

typedef struct phil_status{
    int seat;
    int left_fork;
    int right_fork;
    p_state state;
}p_status;

p_status statuses[NUM_PHILOSOPHERS];

static void initialize_status(p_state state){
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        statuses[i].seat = i;
        statuses[i].left_fork = FALSE;
        statuses[i].right_fork = FALSE;
        statuses[i].state = state;
    }
}

static void update_status(int seat, int left_fork, int right_fork, p_state state){
    //updates the global struct that records the status of all philosophers
    //then prints out the status of all philosophers
    
    //update status
    sem_wait(&printer);
    statuses[seat].seat = seat;
    statuses[seat].left_fork = left_fork;
    statuses[seat].right_fork = right_fork;
    statuses[seat].state = state;
    
    printf("%c:", 'A'+seat);
    for(int i = 0; i< NUM_PHILOSOPHERS; i++){
        p_status cur_status = statuses[i];
        
        char lf = '-';
        char rf = '-';
        if(cur_status.left_fork == TRUE){
            lf = '0' + i; 
        }
        if(cur_status.right_fork == TRUE){
            rf = '0' + (i+1)% NUM_PHILOSOPHERS;
        }
        
        printf("| %c%c%c : %d ", lf, 'A'+i, rf, cur_status.state);
    }
    printf("|\n");
    sem_post(&printer);
}

//-----------------------------------------------------------------------------
static void start_eating(int seat, sem_t *left_fork, sem_t *right_fork){
    /*
    int can_eat = FALSE;
    //check available eaters
    while(can_eat == FALSE){
        sem_wait(&eater);
        if(eaters > 0){
            eaters--;
            can_eat = TRUE;
        }
        sem_post(&eater);
    }
    */
    
    //pick up forks
    if(seat%2 == 0){
        sem_wait(left_fork);
        update_status(seat, TRUE, FALSE, PHIL_CHANGING);
        
        sem_wait(right_fork);
        update_status(seat, TRUE, TRUE, PHIL_CHANGING);
    }else{
        sem_wait(right_fork);
        update_status(seat, FALSE, TRUE, PHIL_CHANGING);
        
        sem_wait(left_fork);
        update_status(seat, TRUE, TRUE, PHIL_CHANGING);
    }
}

static void stop_eating(int seat, sem_t *left_fork, sem_t *right_fork){
    //put down forks
    if(seat%2 == 0){
        sem_post(left_fork);
        update_status(seat, FALSE, TRUE, PHIL_CHANGING);
        
        sem_post(right_fork);
        update_status(seat, FALSE, FALSE, PHIL_CHANGING);
    }else{
        sem_post(right_fork);
        update_status(seat, TRUE, FALSE, PHIL_CHANGING);
        
        sem_post(left_fork);
        update_status(seat, FALSE, FALSE, PHIL_CHANGING);
    }
    //free eaters
    /*
    sem_wait(&eater);
    eaters++;
    sem_post(&eater);
    */
}

void* philosopher(void* arg){
    int seat = ((int*)arg)[0];
    int cycles = ((int*)arg)[1];
    
    sem_t *left_fork = &(forks[seat]);
    //phil 1 has left fork = 0
    sem_t *right_fork = &(forks[(seat+1)% NUM_PHILOSOPHERS]);
    //phil MAX has right fork = 0
    
    p_state state = PHIL_CHANGING;
    p_state next_state = PHIL_EATING;
    
    while(TRUE){
        switch(state){
        case PHIL_CHANGING:
            if(next_state == PHIL_EATING){
                start_eating(seat, left_fork, right_fork);
            }else{//next_state is THINKING or TERM
                stop_eating(seat, left_fork, right_fork);
            }
            state = next_state;
            break;
        case PHIL_EATING:
            update_status(seat, TRUE, TRUE, PHIL_EATING);
            //dawdle();
            cycles--;
            if(cycles > 0){
                next_state = PHIL_THINKING;
            }else{
                next_state = PHIL_TERM;
            }
            
            state = PHIL_CHANGING;
            break;
        case PHIL_THINKING:
            update_status(seat, FALSE, FALSE, PHIL_THINKING);
            //dawdle();
            next_state = PHIL_EATING;
            state = PHIL_CHANGING;
            break;
        case PHIL_TERM:
            update_status(seat, FALSE, FALSE, PHIL_TERM);
            return NULL;
        }
    }
}

int main(int argc, char** argv){
    //determine cycles
    int cycles = 2;
    if(argc > 2){
        printf("Invalid number of arguments\n");
        return 1;
    }else if(argc == 2){
        cycles = (int)argv[1][0];
    }
    
    //create arguments to each philosopher
    //2 args: seat, cycles
    int p_args[NUM_PHILOSOPHERS * 2];
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        p_args[i*2] = i;
        p_args[(i*2)+1] = cycles;
    }
    
    //create philosophers
    pthread_t childid[NUM_PHILOSOPHERS];
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        int res = pthread_create(
            &childid[i],
            NULL,
            philosopher,
            (void*) &(p_args[i*2])
        );
        if(res != 0){
            printf("main:pthread_create error(%d)\n", res);
            return 1;
        }else{
            printf("main:pthread_create good(%c)\n", 'A'+i);
        }
    }
    
    initialize_status(PHIL_CHANGING);
    
    //post semaphores
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        sem_post(&(forks[i]));
    }
    sem_post(&printer);
    sem_post(&eater);
    
    //wait for philosophers to exit
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        pthread_join(childid[i], NULL);
    }
    printf("main:all philosophers ended\n");
    return 0;
}