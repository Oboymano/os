#include<stdio.h>
#include <stdlib.h>
#include<time.h>
#include<string.h>
#include<pthread.h>
#include <unistd.h>

#define MAX_COUNT 100
#define MAX_PRODUCT 5
#define SUPPLIER_COUNT 1
#define CONSUMER_COUNT 1
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
_product* addProduct(char* name_product);
void* dosomething(void*);

time_t timer;
char currentTime[100];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[SUPPLIER_COUNT+CONSUMER_COUNT];

_product product[MAX_PRODUCT];
int currentProduct=0;

int main(){

    int i=0;

    for(i=0;i<SUPPLIER_COUNT;i++){
        _arg *arg  = malloc(sizeof(_arg *));
        arg->id=i+1;
        arg->role='s';
        //printf("wait %d\n",i);
        if(pthread_create( &threads[i], NULL, &dosomething, arg )!=0){
            printf("failed");
        }
        //sleep(1);
    }

    for(i=0;i<CONSUMER_COUNT;i++){
        _arg *arg  = malloc(sizeof(_arg *));
        arg->id=i+1;
        arg->role='c';
        //printf("wait %d\n",i+SUPPLIER_COUNT);
        pthread_create( &threads[i+SUPPLIER_COUNT], NULL, dosomething, arg );
        //sleep(1);
    }

    for(i=0;i<SUPPLIER_COUNT+CONSUMER_COUNT;i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}

char* getTime(){
    time(&timer);
    struct tm * timeinfo = localtime (&timer);
    strftime(currentTime, 50, "%c", timeinfo);
    return currentTime;
}
void* dosomething(void* arg){
    
    _arg *args = arg; 
    _product *current;

    int id = args->id;
    char role = args->role;
    char name_product[257];
    char name_file[100];
    int cond = (role == 's' ? 0 : 1);
    int interval,repeat;

    int waitTime,i=0,tmp_interval=0;

    printf("%s %d\n",getTime(),cond);

    if(!cond){
        sprintf(name_file,"%s%d%s","supplier",id,".txt");
    }else {
        sprintf(name_file,"%s%d%s","consumer",id,".txt");
    }

    FILE *file = fopen(name_file,"r");
    if(!file){
        printf("error file");
    }
    //fprintf(file,"%s\n%d\n%d",name_product,interval,repeat);
    fscanf(file,"%s\n%d\n%d",&name_product,&interval,&repeat);

    pthread_mutex_lock(&mutex);
    current = addProduct(name_product);
    pthread_mutex_unlock(&mutex);

    waitTime=interval;
    
    while(1){

        pthread_mutex_lock(&current->mutex);
        if(!cond&&current->count<MAX_COUNT){
            current->count++;
            printf("%s %s consumed 1 unit. stock after = %d\n",getTime(),current->name,current->count);
            i=0;
            waitTime=interval;
        }else if(cond&&current->count>0){
            current->count--;
            i=0;
            printf("%s %s supplier 1 unit. stock after = %d\n",getTime(),current->name,current->count);
        }else {
            if(i==repeat){
                waitTime*=2;
                i=0;
            }
            if(!cond){
                printf("%s %s supplier going to wait.\n",getTime(),current->name);
            } else {
                printf("%s %s consumer going to wait.\n",getTime(),current->name);
            }
            i++;
            if(waitTime>=60){
                waitTime=60;
            }
        }
        

        pthread_mutex_unlock(&current->mutex);

        sleep(waitTime);
    }

    return NULL;
}
_product *addProduct(char *name){
    int i,now,check=0;
    for(i=0;i<MAX_PRODUCT;i++){
        if(!strcmp(product[i].name,name)){
            now = i;
            check=1;
            return &product[now];
        }
    }
    
    if(currentProduct>=MAX_PRODUCT&&!check){
        printf("error max product\n");
        exit(1);
    }
    if(!check){
        now=currentProduct++;
        strcpy(product[now].name,name);
        product[now].count=0;
        pthread_mutex_init(&product[now].mutex, NULL);
    }

    //printf("%s %s consumed 1 unit. stock after = %d",getTime(),product[now].name,product[now].count);

    return &product[now];    
}
