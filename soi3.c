#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/sem.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "keys.h"
#define SIZE_OF_QUEUE 7

/* SEMUN UNION */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};

/* GLOBAL VARIABLES */
int *queue1;
int *queue2;
int semQ1FULL, semQ1EMPTY, semQ1MUTEX, semQ2FULL, semQ2EMPTY, semQ2MUTEX;

/* NECCESSARY FUNCTIONS' DEFINITIONS */
int semaphore_initialize (int semid, int value);

int semaphore_allocate (key_t key, int sem_flags);

int semaphore_deallocate (int semid);

void allocate_semaphores();

void deallocate_all();

void semaphore_down(int semid);

void semaphore_up (int semid);

int semaphore_value(int semid);

void allocate_queues();

int queue_append(int *queue, int value);

int queue_popfirst(int *queue);

int queue_getsize(int *queue);

void queue_print(int *queue);

void error(char *msg);

/* THREADS */
void* producentA(void *a)
{
    int i;
    for(i=0;i<100;i++)
    {
        semaphore_down(semQ1EMPTY);
        semaphore_down(semQ1MUTEX);
        queue_append(queue1, i);
        printf("PRODUCENT1 APPENDED: %d | QUEUE1: ", i);
        printf("sQ1F=%d|sQ1M=%d|sQ1E=%d\n", semaphore_value(semQ1FULL), semaphore_value(semQ1MUTEX), semaphore_value(semQ1EMPTY));
        queue_print(queue1);
        semaphore_up(semQ1FULL);
        semaphore_up(semQ1MUTEX);
        sleep((rand() % 3));
    }
    return NULL;
}

void* producentB(void *a)
{
    int i=0;
    for(i=0;i<100;i++)
    {
        semaphore_down(semQ2EMPTY);
        semaphore_down(semQ2MUTEX);
        queue_append(queue2, i*100);
        printf("PRODUCENT2 APPENDED: %d | QUEUE2: ", i*100);
        printf("sQ2F=%d|sQ2M=%d|sQ2E=%d\n", semaphore_value(semQ2FULL), semaphore_value(semQ2MUTEX), semaphore_value(semQ2EMPTY));
        queue_print(queue2);
        semaphore_up(semQ2FULL);
        semaphore_up(semQ2MUTEX);
        sleep((rand() % 3));
    }
    return NULL;
}

void* consumentA (void *a)
{
    int actual_queue = 0;
    int element;
    for(int loop=0; loop<100; ++loop){
        if(actual_queue == 0){
            semaphore_down(semQ1FULL);
            semaphore_down(semQ1MUTEX);
            element = queue_popfirst(queue1);
            printf("CONSUMERA -> POPPED ELEMENT= %d | Queue1: ", element);
            printf("sQ1F=%d|sQ1M=%d|sQ1E=%d\n", semaphore_value(semQ1FULL), semaphore_value(semQ1MUTEX), semaphore_value(semQ1EMPTY));
            queue_print(queue1);
            semaphore_up(semQ1EMPTY);
            semaphore_up(semQ1MUTEX);
            actual_queue++;
        }
        else{
            semaphore_down(semQ2FULL);
            semaphore_down(semQ2MUTEX);
            element = queue_popfirst(queue2);
            printf("CONSUMERA -> POPPED ELEMENT= %d | Queue2: ", element);
            printf("sQ2F=%d|sQ2M=%d|sQ2E=%d\n", semaphore_value(semQ2FULL), semaphore_value(semQ2MUTEX), semaphore_value(semQ2EMPTY));
            queue_print(queue2);
            semaphore_up(semQ2MUTEX);
            semaphore_up(semQ2EMPTY);
            actual_queue--;
        }
        sleep((rand() % 3));
    }
    return NULL;
}

void* consumentB (void *a)
{
    int actual_queue = 1;
    int element;
    for(int loop=0; loop<100; ++loop){
        if(actual_queue == 0){
            semaphore_down(semQ1FULL);
            semaphore_down(semQ1MUTEX);
            element = queue_popfirst(queue1);
            printf("CONSUMERB -> POPPED ELEMENT= %d | Queue1: ", element);
            printf("sQ1F=%d|sQ1M=%d|sQ1E=%d\n", semaphore_value(semQ1FULL), semaphore_value(semQ1MUTEX), semaphore_value(semQ1EMPTY));
            queue_print(queue1);
            semaphore_up(semQ1EMPTY);
            semaphore_up(semQ1MUTEX);
            actual_queue++;
        }
        else{
            semaphore_down(semQ2FULL);
            semaphore_down(semQ2MUTEX);
            element = queue_popfirst(queue2);
            printf("CONSUMERB -> POPPED ELEMENT= %d | Queue2: ", element);
            printf("sQ2F=%d|sQ2M=%d|sQ2E=%d\n", semaphore_value(semQ2FULL), semaphore_value(semQ2MUTEX), semaphore_value(semQ2EMPTY));
            queue_print(queue2);
            semaphore_up(semQ2MUTEX);
            semaphore_up(semQ2EMPTY);
            actual_queue--;
        }
        sleep((rand() % 3));
    }
}
void run_test1(){
    pthread_t t0;
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;
 
    if(pthread_create(&t0,NULL,producentA,NULL)==-1)
        error("Nie mozna utworzyc watku t0");
    if(pthread_create(&t1,NULL,producentB,NULL)==-1)
        error("Nie mozna utworzyc watku t1");
    if(pthread_create(&t2,NULL,consumentA,NULL)==-1)
        error("Nie mozna utworzyc watku t1");
    if(pthread_create(&t3,NULL,consumentB,NULL)==-1)
        error("Nie mozna utworzyc watku t1");
 
    void* result;
    if(pthread_join(t0,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
    if(pthread_join(t1,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
    if(pthread_join(t2,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
    if(pthread_join(t3,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
}
void run_test2(){
    pthread_t t0;
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;
 
    if(pthread_create(&t0,NULL,producentA,NULL)==-1)
        error("Nie mozna utworzyc watku t0");
    if(pthread_create(&t1,NULL,producentB,NULL)==-1)
        error("Nie mozna utworzyc watku t1");
    
    void* result;
    if(pthread_join(t0,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
    if(pthread_join(t1,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
}
void run_test3(){
    pthread_t t2;
    pthread_t t3;
 
    if(pthread_create(&t2,NULL,consumentA,NULL)==-1)
        error("Nie mozna utworzyc watku t1");
    if(pthread_create(&t3,NULL,consumentB,NULL)==-1)
        error("Nie mozna utworzyc watku t1");
 
    void* result;
    if(pthread_join(t2,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
    if(pthread_join(t3,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
}
void run_test4(){
    pthread_t t0;
    pthread_t t2;
    
    if(pthread_create(&t0,NULL,producentA,NULL)==-1)
        error("Nie mozna utworzyc watku t0");
    if(pthread_create(&t2,NULL,consumentA,NULL)==-1)
        error("Nie mozna utworzyc watku t1");
    
    void* result;
    if(pthread_join(t0,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
    if(pthread_join(t2,&result)==-1)
        error("Blad oczekiwania na zakonczenie watku t0");
    }

int main(){
    time_t t;
    srand((unsigned) time(&t));
    allocate_semaphores();
    allocate_queues();
    run_test1();
    // run_test2();
    // run_test3();
    // run_test4();
    
    
    deallocate_all();
    return 0;
}

void allocate_semaphores(){
    semQ1FULL = semaphore_allocate(semQ1_FULL_key, 0666|IPC_CREAT);
    semQ1EMPTY = semaphore_allocate(semQ1_EMPTY_key, 0666|IPC_CREAT);
    semQ1MUTEX = semaphore_allocate(semQ1_MUTEX_key, 0666|IPC_CREAT);
    semQ2FULL = semaphore_allocate(semQ2_FULL_key, 0666|IPC_CREAT);
    semQ2EMPTY = semaphore_allocate(semQ2_EMPTY_key, 0666|IPC_CREAT);
    semQ2MUTEX = semaphore_allocate(semQ2_MUTEX_key, 0666|IPC_CREAT);
    semaphore_initialize(semQ1FULL, 0);
    semaphore_initialize(semQ1EMPTY, SIZE_OF_QUEUE);
    semaphore_initialize(semQ1MUTEX, 1);
    semaphore_initialize(semQ2FULL, 0);
    semaphore_initialize(semQ2EMPTY, SIZE_OF_QUEUE);
    semaphore_initialize(semQ2MUTEX, 1);
    }

void deallocate_all(){
    semaphore_deallocate(semQ1FULL);
    semaphore_deallocate(semQ1EMPTY);
    semaphore_deallocate(semQ1MUTEX);
    semaphore_deallocate(semQ2FULL);
    semaphore_deallocate(semQ2EMPTY);
    semaphore_deallocate(semQ2MUTEX);
    free(queue1);
    free(queue2);
}

void semaphore_down(int semid){
    struct sembuf operation = {0, -1, SEM_UNDO};
    semop (semid, &operation, 1);
}

void semaphore_up (int semid){
    struct sembuf operation={0, 1, SEM_UNDO};
    semop (semid, &operation, 1);
}

int semaphore_value(int semid){
    return semctl(semid, 0, GETVAL, 0);
}

int queue_append(int *queue, int value){
    if (queue[0] >= SIZE_OF_QUEUE)
    {
        return -1;
    }
    queue[queue[0] + 1] = value;
    queue[0]++;
    return 0;
}

int queue_popfirst(int *queue){
    if(queue[0]<=0){
        return -1;
    }
    int returned_element = queue[1];
    for(int i=1; i<=queue[0];++i){
        queue[i] = queue[i+1];
    }
    queue[0]--;
    return returned_element;
}

int queue_getsize(int *queue){
    return queue[0];
}

void allocate_queues()
{
    queue1 = malloc((SIZE_OF_QUEUE+1)*sizeof(int));
    queue2 = malloc((SIZE_OF_QUEUE+1)*sizeof(int));
    queue1[0] = 0;
    queue2[0] = 0;
}

int semaphore_initialize (int semid, int value)
{
    union semun argument;
    unsigned short values[1];
    values[0] = value;
    argument.array = values;
    return semctl (semid, 0, SETALL, argument);
}

int semaphore_allocate (key_t key, int sem_flags)
{
    return semget (key, 1, sem_flags);
}

int semaphore_deallocate (int semid)
{
    union semun ignored_argument;
    return semctl (semid, 1, IPC_RMID, ignored_argument);
}

void queue_print(int *queue){
    for(int i = 1; i<=queue[0]; ++i)
    {
        printf("%d->", queue[i]);
    }
    printf("\n");
}


void error(char *msg)
{
    fprintf(stderr,"%s:%s\n",msg,strerror(errno));
    exit(1);
}
