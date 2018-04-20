#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// max good amount
#define MAX_GOOD 5

// max amount per good
#define MAX_AMOUNT 100

// supplier amount
#define SUPPLIER_AMOUNT 2

// consumer amount
#define CONSUMER_AMOUNT 2

typedef struct good
{
    char name[257];
    int amount;
    pthread_mutex_t mutex;
} good_t;

typedef struct arg
{
    int id;
    char role;                  // 'S' = supplier , 'C' = consumer
} arg_t;

// function prototype
char *cctime();
good_t *get_good_or_create(char *);
void *entry(void *);

// pthread variable
pthread_t threads[SUPPLIER_AMOUNT + CONSUMER_AMOUNT];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// time variable
time_t now;
char s_time[50];

// goods
good_t goods[MAX_GOOD];
int good_count = 0;

int main()
{
    int i, j;

    printf("%s STARTING...\nSupplier amount: %d\nConsumer amount: %d\n-------------------------------------------\n\n", cctime(), SUPPLIER_AMOUNT, CONSUMER_AMOUNT);

    // create supplier thread
    for (i = 0; i < SUPPLIER_AMOUNT; i++)
    {
        arg_t *arg = malloc(sizeof(arg_t *));
        arg->id = i + 1;
        arg->role = 'S';
        if (pthread_create(&threads[i], NULL, &entry, arg) != 0)
        {
            printf("Error creating supplier thread %d!\n", i + 1);
        }
    }

    // create consumer thread
    for (j = 0; j < CONSUMER_AMOUNT; i++, j++)
    {
        arg_t *arg = malloc(sizeof(arg_t *));
        arg->id = j + 1;
        arg->role = 'C';
        if (pthread_create(&threads[i], NULL, &entry, arg) != 0)
        {
            printf("Error creating consumer thread %d!\n", j + 1);
        }
    }

    // infinite loop
    while (1);

    return 0;
}

// get time and return as string
char *cctime()
{
    time(&now);
    struct tm *p = localtime(&now);
    strftime(s_time, 50, "%c", p);
    return s_time;
}

// get good or create if not exist
good_t *get_good_or_create(char *name)
{
    int i;
    
    // check if good is exist then return
    for (i = 0; i < good_count; i++)
    {
        if (!strcmp(goods[i].name, name))
        {
            return &goods[i];
        }
    }

    // good is not exist
    // check if good amount exceed MAX_GOOD
    if (good_count >= MAX_GOOD)
    {
        fprintf(stderr, "Good amount exceed maximum number of good!");
        exit(1);
    }

    // add information
    strcpy(goods[good_count].name, name);
    goods[good_count].amount = 0;
    pthread_mutex_init(&goods[good_count].mutex, NULL);

    // increase good count
    good_count++;

    return &goods[good_count - 1];
}

void *entry(void *arg)
{
    char tmp_file[15], tmp_name[257];
    good_t *good;
    int interval, repeat;

    // get id & person role
    arg_t *arg_r = arg;
    int id = arg_r->id;
    char role = arg_r->role;

    // read from file
    sprintf(tmp_file, "%s%d.txt", (role == 'S' ? "supplier" : "consumer"), id);
    FILE *fp = fopen(tmp_file, "r");

    // check if file is exist
    if (!fp)
    {
        fprintf(stderr, "Cannot read from file %s!", tmp_file);
        exit(1);
    }

    // read good name
    fscanf(fp, "%[^\n]", tmp_name);

    // get good
    pthread_mutex_lock(&mutex);
    good = get_good_or_create(tmp_name);
    pthread_mutex_unlock(&mutex);

    // read other information
    fscanf(fp, "%d\n%d", &interval, &repeat);

    // check whether interval and repeat is <= zero or not
    if (interval <= 0 || repeat <= 0)
    {
        fprintf(stderr, "Interval or repeat must greater than zero in file %s!", tmp_file);
        exit(1);
    }

    // attempt count
    int i;
    // time to wait (default is interval)
    int time_to_wait = interval;
    for (i = 1; ; i++)
    {
        // lock thread
        pthread_mutex_lock(&good->mutex);
        
        // check if amount is ok
        if ((role == 'S' && good->amount < MAX_AMOUNT) || (role == 'C' && good->amount > 0))
        {
            // increase/decrease amount of good
            good->amount += (role == 'S' ? 1 : -1);

            printf("%s %s %s 1 unit. stock after = %d\n", cctime(), good->name, (role == 'S' ? "supplied" : "consumed"), good->amount);

            // reset time to wait and attempt count
            time_to_wait = interval;
            i = 0;
        }
        else
        {
            printf("%s %s %s going to wait.\n", cctime(), good->name, (role == 'S' ? "supplier" : "consumer"));

            // if attempt = repeat
            if (i == repeat)
            {
                // reset attempt count
                i = 0;
                // multiple time to wait by 2
                time_to_wait *= 2;
                // check if its exceed 60 sec
                if (time_to_wait >= 60)
                    time_to_wait = 60;
            }
        }

        // unlock thread
        pthread_mutex_unlock(&good->mutex);

        // wait
        sleep(time_to_wait);
    }

    return NULL;
}