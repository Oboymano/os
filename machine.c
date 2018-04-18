#include<stdio.h>
#include <stdlib.h>
#include<time.h>
#include<string.h>
#include<pthread.h>

#define MAX_COUNT 100
#define MAX_PRODUCT 5
#define SUPPLIER_COUNT 2
#define CONSUMER_COUNT 2
typedef struct product {
    char name[257];
    int count;
    pthread_mutex_t mutex;
}_product;
typedef struct arg {
    int id;
    char role;
}_arg;
char* getTime();
void* dosomething(void*);

time_t timer;
char currentTime[100];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[SUPPLIER_COUNT+CONSUMER_COUNT];

_product product[MAX_PRODUCT];

int main(){

    int i=0;
    _arg *arg  = malloc(sizeof(_arg *));

    for(i=0;i<SUPPLIER_COUNT;i++){
        arg->id=i;
        arg->role='s';
        printf("wait %d\n",i);
        if(pthread_create( &threads[i], NULL, &dosomething, arg )!=0){
            printf("failed");
        }
    }

    for(i=0;i<CONSUMER_COUNT;i++){
        arg->id=i;
        arg->role='c';
        printf("wait %d\n",i);
        pthread_create( &threads[i+SUPPLIER_COUNT], NULL, dosomething, arg );
    }

    return 0;
}
/*
char* getTime(){
    
    time(&timer);
    struct tm * timeinfo = localtime (&timer);
    strftime(currentTime, 50, "%c", timeinfo);
    return "currentTime";
}
*/
void* dosomething(void* arg){
    
    _arg *args = arg; 

    int id = args->id;
    char role = args->role;
    char name_product[257];
    char name_file[100];
    int cond = (role == 's' ? 0 : 1);
    int interval,repeat;

    int waitTime;

    if(!cond){
        sprintf(name_file,"%s%d%s","supplier",id,".txt");
    }else {
        sprintf(name_file,"%s%d%s","consumer",id,".txt");
    }

    FILE *file = fopen(name_file,"r");
    fprintf(file,"%s\n%d\n%d",name_product,interval,repeat);

    printf("%d %s \n",id,name_file);
    if(!cond){
        
    }

    return NULL;
}