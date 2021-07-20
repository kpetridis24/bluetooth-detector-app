#ifndef __BT_APP_H__
#define __BT_APP_H__

#include "utils.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

pthread_mutex_t BTmutex, COVmutex, CONTACTmutex, CLEARmutex;
pthread_cond_t BTcond, COVcond, CONTACTcond, CLEARcond;

bool isPositive = false;
bool BTsearch = false;
bool COVtest = false;
bool clearContacts = false;
bool terminate = false;

char addresses[300][12];
double realBTtimer[3000];


/********************************************
*****            Structures             *****
*********************************************/

typedef struct {

    char code[NUM_CHARS];
    struct timeval selfInitTime;

} macaddress;


typedef struct {

  macaddress buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
  
} queue;


typedef struct {

    queue *tempAddress;
    queue *closeContacts;

} param;


/********************************************
*****   Queue functions implementation  *****
*********************************************/

queue *queueInit (void)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);
	
  return (q);
}

void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);	
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

void queueAdd (queue *q, macaddress in)
{
  q->buf[q->tail] = in;
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;

  return;
}

void queueDel (queue *q, macaddress *out)
{
  *out = q->buf[q->head];

  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  return;
}


/********************************************
*****   App functions implementation    *****
*********************************************/

void *clearCloseContacts(void *q){

    param *queues = (param *)q;
    queue *fifo = (queue *)(queues->tempAddress);
    queue *closeContactList = (queue *)(queues->closeContacts);

    while(1)
    {
        pthread_mutex_lock(&CLEARmutex);
        while(!(clearContacts || terminate))
            pthread_cond_wait(&CLEARcond, &CLEARmutex);
        
        pthread_mutex_unlock(&CLEARmutex);
        
        if(terminate) pthread_exit(NULL);

        printf("Clearing close contacts...\n");
        closeContactList->head = 0;
        closeContactList->tail = 0;
        closeContactList->empty = 1;
        clearContacts = false;
    }
}


void *uploadContacts(void *q){
    
    param *queues = (param *)q;
    queue *fifo = (queue *)(queues->tempAddress);
    queue *closeContactList = (queue *)(queues->closeContacts);

    while(1)
    {   
        pthread_mutex_lock(&CONTACTmutex);
        while(!(isPositive || terminate))
            pthread_cond_wait(&CONTACTcond, &CONTACTmutex);
        
        pthread_mutex_unlock(&CONTACTmutex);

        FILE *fp = fopen("covidTrace.bin", "a+");
        if(terminate){
            //fclose(fp);
            pthread_exit(NULL);
        }

        //printf("Uploading close contacts...\n");

        for(int i = closeContactList->head; i < closeContactList->tail; i++){
            for(int k = 0; k < NUM_CHARS; k++)
                fwrite(&closeContactList->buf[i].code[k], sizeof(char), 1, fp);
            fprintf(fp, "\n");
        }
        
        clearContacts = true;
        pthread_cond_signal(&CLEARcond);
        fclose(fp);
        isPositive = false;
    }
}


void *testCOVID(void *args){

    while(1)
    {   
        pthread_mutex_lock(&COVmutex);
        while(!(COVtest || terminate))
            pthread_cond_wait(&COVcond, &COVmutex);
        
        pthread_mutex_unlock(&COVmutex);

        if(terminate) pthread_exit(NULL);

        isPositive = rand() % 2;
        printf("COVID test: %d\n", isPositive);
        if(isPositive)
            pthread_cond_signal(&CONTACTcond);
        
        COVtest = false;
    }
}


void *eventTimer(void *args){

    struct timeval timeInit1 = tic();
    struct timeval timeInit2 = tic();
    struct timeval timeInit3 = tic();
    int count = 0;

    while(1)
    {   
        
        if(toc(timeInit1) >= BT_RATE){          //Bluetooth search.

            printf("BT search...\n");
            BTsearch = true;
            realBTtimer[count++] = toc(timeInit1);
            pthread_cond_signal(&BTcond);
            timeInit1 = tic();
        }

        if(toc(timeInit2) >= COV_RATE){        //Covid test. 

            COVtest = true;
            pthread_cond_signal(&COVcond);
            timeInit2 = tic();
        }

        if(toc(timeInit3) >= TERMINATION_TIME){
            
            terminate = true;
            pthread_cond_signal(&BTcond);
            pthread_cond_signal(&COVcond);
            pthread_cond_signal(&CLEARcond);
            pthread_cond_signal(&CONTACTcond);
            
            FILE *fp = fopen("realBTtimer.txt", "r+");
            for(int i = 0; i < count; i++) fprintf(fp, "%f\n", realBTtimer[i]);
            pthread_exit(NULL);
        }
    }
}


void *BTnearMe(void *q){
    
    param *queues = (param *)q;
    queue *fifo = (queue *)(queues->tempAddress);
    queue *closeContactList = (queue *)(queues->closeContacts);
    srand(time(NULL));
    double dur;

    while(1)
    {   
        pthread_mutex_lock(&BTmutex);
        while(!(BTsearch || terminate))
            pthread_cond_wait(&BTcond, &BTmutex);
        
        pthread_mutex_unlock(&BTmutex);
        BTsearch = false;
        
        if(terminate) pthread_exit(NULL);
        
        /* Delete contacts after some time. */
        if(fifo->tail > 0 && toc(fifo->buf[fifo->head].selfInitTime) >= NON_CLOSE_CONTACT_DUR){
            macaddress *out = (macaddress *)malloc(sizeof(macaddress));
            queueDel(fifo, out);
            free(out);
        }

        /* Delete close contacts after some time. */
        if(closeContactList->tail > 0 && toc(closeContactList->buf[closeContactList->head].selfInitTime) >= CLOSE_CONTACT_DUR){
            macaddress *out = (macaddress *)malloc(sizeof(macaddress));
            queueDel(closeContactList, out);
            free(out);
        }

        int addressIndex = rand() % NUM_ADDRESSES;
        macaddress addr;

        for(int i = 0; i < NUM_CHARS; i++)
           addr.code[i] = addresses[addressIndex][i];
        
        addr.selfInitTime = tic();
        queueAdd(fifo, addr);

        for(int j = fifo->head; j < fifo->tail - 1; j++){
            if(isSameCode(fifo->buf[j].code, addr.code)){

                dur = toc(fifo->buf[j].selfInitTime);
                if(dur >= 2 && dur <= 50){  //It should be 240 and 1200

                    fifo->tail--;
                    queueAdd(closeContactList, addr);
                    printf("Close contact detected!\n");
                }
            }
        }
    }
}


#endif