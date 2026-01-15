#include <stdio.h>
#include <stdlib.h>

void fill(int *p, size_t size){
    for(int i = 0; i < size/sizeof(int); i++){
        p[i] = i;
    }
}

int test_fill(int *p, size_t size){
    for(int i = 0; i < size/sizeof(int); i++){
        if(p[i] != i){
            return 0;
        }
    }
    return 1;
}

void test_dump(int *p, size_t size){
    printf("test:dump ");
    for(int i = 0; i < size/sizeof(int); i++){
        printf("|%d", p[i]);
    }
    printf("\n");
}

int main(){
    printf("test: running\n");
    int *loc1 = malloc(sizeof(int)*1000);
    int *loc2 = malloc(sizeof(int)*1000);
    int *loc3 = malloc(sizeof(int)*1000);
    int *loc4 = malloc(sizeof(int)*1000);
    int *loc5 = malloc(sizeof(int)*1000);
    int *loc6 = malloc(sizeof(int)*1000);
    int *loc7 = malloc(sizeof(int)*1000);
}