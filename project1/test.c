#include <stdio.h>
#include <stdlib.h>

void fill(int *p, size_t size){
    int i;
    for(i = 0; i < size/sizeof(int); i++){
        p[i] = i;
    }
}

int test_fill(int *p, size_t size){
    int i;
    for(i = 0; i < size/sizeof(int); i++){
        if(p[i] != i){
            return 0;
        }
    }
    return 1;
}

void test_dump(int *p, size_t size){
    printf("test:dump ");
    int i;
    for(i = 0; i < size/sizeof(int); i++){
        printf("|%d", p[i]);
    }
    printf("\n");
}

int main(){
    printf("test: running\n");
    malloc(105);
    return 0;
}