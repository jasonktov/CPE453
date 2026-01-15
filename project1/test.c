#include <stdio.h>

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
    int *loc1 = malloc(sizeof(int)*10);
    fill(loc1, sizeof(int) * 10);
    int *loc2 = malloc(sizeof(int)*100000);
    test_dump(loc1, sizeof(int)*10);
    printf("test: loc1[%p] = %d\n", loc1, test_fill(loc1, sizeof(int) * 10));
    
    loc1 = realloc(loc1, sizeof(int)*5);
    test_dump(loc1, sizeof(int)*5);
    printf("test: expand loc1[%p] = %d\n", loc1, test_fill(loc1, sizeof(int) * 5));
    
    loc1 = realloc(loc1, sizeof(int)*5);
    test_dump(loc1, sizeof(int)*5);
    printf("test: expand loc1[%p] = %d\n", loc1, test_fill(loc1, sizeof(int) * 5));
}