#include<stdio.h>
#include <stdlib.h>
#include<time.h>
#include<string.h>
#include<pthread.h>
#include <unistd.h>

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
        pthread_create( &threads[i], NULL, &dosomething, arg );
            
    }

    for(i=0;i<CONSUMER_COUNT;i++){
        _arg *arg  = malloc(sizeof(_arg *));
        arg->id=i+1;
        arg->role='c';
        pthread_create( &threads[i+SUPPLIER_COUNT], NULL, dosomething, arg );
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
    int cond = (role == 's' ? 0 : 1);                               // if (role = supplier)  cond = 0 else cond = 1 
    int interval,repeat;

    int waitTime,waitCount=0;
    // set name of file to read
    if(!cond){
        sprintf(name_file,"%s%d%s","supplier",id,".txt");           // supplier .txt   
    }else {
        sprintf(name_file,"%s%d%s","consumer",id,".txt");           // consumer .txt
    }

    FILE *file = fopen(name_file,"r");                              // open file
    if(!file){
        printf("error file %s!",name_file);                         // file doesn't exist 
    }
    fscanf(file,"%s\n%d\n%d",&name_product,&interval,&repeat);      // read parameter in file (name of product,interval,repeat)

    pthread_mutex_lock(&mutex);                                     // mutex lock for check product
    current = addProduct(name_product);
    pthread_mutex_unlock(&mutex);                                   // mutex unlock then finished

    waitTime=interval;                                              // default wait time is interval
    // loop infinite
    while(1){
        // product mutex lock 
        pthread_mutex_lock(&current->mutex);
        if(!cond&&current->count<MAX_COUNT){                        // supplier condition
            current->count++;                                       // increase amount of product
            printf("%s %s consumed 1 unit. stock after = %d\n",getTime(),current->name,current->count);
            waitCount=0;                                            // reset wait count
            waitTime=interval;                                      // reset wait time = interval
        }else if(cond&&current->count>0){                           // consumer condition
            current->count--;                                       // decrease amount of product
            printf("%s %s supplier 1 unit. stock after = %d\n",getTime(),current->name,current->count);
            waitCount=0;                                            // reset wait count
            waitTime=interval;                                      // reset wait time = interval
        }else {                                                     // going to wait
            // wait count = repeat then wait time multiply by 2
            if(waitCount==repeat){                                  
                waitTime*=2;
                waitCount=0;
            }
            // show supplier or consumer going to wait
            if(!cond){
                printf("%s %s supplier going to wait.\n",getTime(),current->name);
            } else {
                printf("%s %s consumer going to wait.\n",getTime(),current->name);
            }
            waitCount++;
            // max wait time = 60
            if(waitTime>=60){
                waitTime=60;
            }
        }
        pthread_mutex_unlock(&current->mutex);                      // product mutex unlock 

        sleep(waitTime);                                            // wait for next action 
    }
    
}
// init product
_product *addProduct(char *name){
    int i,now,check=0;
    for(i=0;i<MAX_PRODUCT;i++){
        // product already then send address of product
        if(!strcmp(product[i].name,name)){
            now = i;
            check=1;
            return &product[now];                       // return address of product
        }
    }
    // product over
    if(currentProduct>=MAX_PRODUCT&&!check){
        printf("error max product\n");
        exit(1);
    }
    // product doesn't exist then add product and send address of product
    if(!check){
        now=currentProduct++;                           // increase number of product
        strcpy(product[now].name,name);                 // product name
        product[now].count=0;                           // amount of product = 0
        pthread_mutex_init(&product[now].mutex, NULL);  // initial mutex of product
    }
    return &product[now];                               // return address of product
}
