#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_CHILDREN 2

sem_t sem;
int shared_value;

void *child(void* id){
    int whoami=*(int*)id;
    printf("child %d [%d]\n", whoami, getpid());
    return NULL;
}

void *downer(void* arg){
    printf("downer:waiting for sem\n");
    sem_wait(&sem);
    printf("downer:downed sem, value=%d\n", shared_value);
    shared_value -= *(int*)arg;
    
    sem_post(&sem);
    printf("downer:upped sem, value=%d\n", shared_value);
    return NULL;
}

void *upper(void* arg){
    printf("upper:waiting for sem\n");
    sem_wait(&sem);
    printf("upper:downed sem, value=%d\n", shared_value);
    shared_value += *(int*)arg;
    
    sem_post(&sem);
    printf("upper:upped sem, value=%d\n", shared_value);
    return NULL;
}

int main(){
    printf("main: %d children\n", NUM_CHILDREN);
    printf("Philosopher %c\n", 'A' - 1 + 2);
    
    //int id[NUM_CHILDREN];
    pthread_t childid[NUM_CHILDREN];
    int up_val = 10;
    int down_val = 3;
    
    if(sem_init(&sem, 0, 0) == -1){
        printf("main:sem failed init\n");
        return 1;
    }
    
    
    pthread_create(
        &childid[0],
        NULL,
        upper,
        (void*) &up_val
    );
    
    pthread_create(
        &childid[1],
        NULL,
        downer,
        (void*) &down_val
    );
    
    /* for(int i = 0; i < NUM_CHILDREN; i++){
        id[i] = i;
        
        int res;
        res = pthread_create(
                &childid[i],
                NULL,
                child,
                (void*) &id[i]
                );
        printf("main: spawned child %d [%ld]\n", id[i], childid[i]);
    } */
    sem_post(&sem);
    for(int i=0;i<NUM_CHILDREN;i++){
        printf("main:waiting to join child %d [%ld]\n", i, childid[i]);
        pthread_join(childid[i], NULL);
        printf("main:joined child %d [%ld]\n", i, childid[i]);
    }
    
    return 0;
}