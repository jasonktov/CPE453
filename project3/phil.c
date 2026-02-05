#include "phil.h"

sem_t forks[NUM_PHILOSOPHERS];
sem_t printer;

p_status statuses[NUM_PHILOSOPHERS];

static int str_to_int(char* str){
    int n = 0;
    int i = 0;
    while(str[i] != '\0'){
        n *= 10;
        n += str[i] - '0';
        i++;
    }
    return n;
}

static void initialize_status(p_state state){
    //initialize all philosophers to no forks, and state
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        statuses[i].left_fork = NO_FORK;
        statuses[i].right_fork = NO_FORK;
        statuses[i].state = state;
    }
}

static void print_header(){
    //line 1
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        printf("|=");
        for(int f = 0; f < NUM_PHILOSOPHERS; f++){
            printf("=");
        }
        printf("=======");
    }
    printf("|\n");
    
    //line 2
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        printf("| ");
        for(int f = 0; f < NUM_PHILOSOPHERS; f++){
            printf(" ");
        }
        printf("%c      ", 'A'+i);
    }
    printf("|\n");
    
    //line 3
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        printf("|=");
        for(int f = 0; f < NUM_PHILOSOPHERS; f++){
            printf("=");
        }
        printf("=======");
    }
    printf("|\n");
}

static void update_status(int seat,int left_fork,int right_fork,p_state state){
    //updates the global struct that records the status of all philosophers
    //then prints out the status of all philosophers
    
    //update status
    sem_wait(&printer);
    statuses[seat].left_fork = left_fork;
    statuses[seat].right_fork = right_fork;
    statuses[seat].state = state;
    
    //print the status line
    char state_str[STATE_STR_LEN];
    int lf_i = -1;
    int rf_i = -1;
    char forks[NUM_PHILOSOPHERS+1];
    
    for(int i = 0; i< NUM_PHILOSOPHERS; i++){
        p_status cur_phil = statuses[i];
        
        //determine state
        if(cur_phil.state == PHIL_EATING){
            strncpy(state_str, EAT_STR, STATE_STR_LEN);
        }else if(cur_phil.state == PHIL_THINKING){
            strncpy(state_str, THINK_STR, STATE_STR_LEN);
        }else{
            strncpy(state_str, CHANGE_STR, STATE_STR_LEN);
        }
        
        //determine forks
        lf_i = -1;
        rf_i = -1;
        if(cur_phil.left_fork == TRUE){
            lf_i = i;
        }
        if(cur_phil.right_fork == TRUE){
            rf_i = (i+1)%NUM_PHILOSOPHERS;
        }
        for(int f = 0; f < NUM_PHILOSOPHERS; f++){
            if(f == lf_i || f == rf_i){
                forks[f] = '0' + f;
            }else{
                forks[f] = '-';
            }
        }
        forks[NUM_PHILOSOPHERS] = '\0';//string terminator
            
        printf("| %s %s ", forks, state_str);
    }
    printf("|\n");
    sem_post(&printer);
}

//-----------------------------------------------------------------------------
static void start_eating(int seat, sem_t *left_fork, sem_t *right_fork){
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
        //evens pick up left fork first
        sem_post(left_fork);
        update_status(seat, FALSE, TRUE, PHIL_CHANGING);
        
        sem_post(right_fork);
        update_status(seat, FALSE, FALSE, PHIL_CHANGING);
    }else{
        //odds pick up right fork first
        sem_post(right_fork);
        update_status(seat, TRUE, FALSE, PHIL_CHANGING);
        
        sem_post(left_fork);
        update_status(seat, FALSE, FALSE, PHIL_CHANGING);
    }
}

void* philosopher(void* arg){
    int seat = ((int*)arg)[0];
    int cycles = ((int*)arg)[1];
    
    sem_t *left_fork = &(forks[seat]);
    sem_t *right_fork = &(forks[(seat+1)% NUM_PHILOSOPHERS]);
    
    p_state state = PHIL_CHANGING;
    p_state next_state = PHIL_EATING;
    
    while(TRUE){
        switch(state){
        case PHIL_CHANGING:
            if(next_state == PHIL_EATING){
                update_status(seat, FALSE, FALSE, PHIL_CHANGING);
                start_eating(seat, left_fork, right_fork);
            }else if(next_state == PHIL_THINKING){
                update_status(seat, TRUE, TRUE, PHIL_CHANGING);
                stop_eating(seat, left_fork, right_fork);
            }else{
                update_status(seat, FALSE, FALSE, PHIL_CHANGING);
            }
            state = next_state;
            break;
            
        case PHIL_EATING:
            update_status(seat, TRUE, TRUE, PHIL_EATING);
            dawdle();
            
            next_state = PHIL_THINKING;
            state = PHIL_CHANGING;
            break;
            
        case PHIL_THINKING:
            update_status(seat, FALSE, FALSE, PHIL_THINKING);
            dawdle();
            cycles--;
            if(cycles > 0){
                next_state = PHIL_EATING;
            }else{
                next_state = PHIL_TERM;
            }
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
    int cycles = 1;
    if(argc > 2){
        printf("Invalid number of arguments\n");
        return 1;
    }else if(argc == 2){
        cycles = str_to_int(argv[1]);
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
        }
    }
    
    //print start of status message
    initialize_status(PHIL_CHANGING);
    print_header();
    
    //post semaphores
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        sem_post(&(forks[i]));
    }
    sem_post(&printer);
    
    //wait for philosophers to exit
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        pthread_join(childid[i], NULL);
    }
    return 0;
}
