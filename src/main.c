#include "BTapp.h"
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>


/********************************************
*****              main                 *****
*********************************************/

int main(){

    /* Read addresses from file. */
    FILE *fp = fopen("addr.txt", "r");

    for(int i = 0; i < NUM_ADDRESSES; i++)
        for(int j = 0; j < NUM_CHARS; j++)
            fscanf(fp, "%c ", &addresses[i][j]);

    fclose(fp);
    
    /* Run the app. */
    param *queues = (param *)malloc(sizeof(param));
    queues->tempAddress = queueInit();
    queues->closeContacts = queueInit();

    int numWorkers = 5;
    pthread_t *thread = (pthread_t *)malloc(numWorkers * sizeof(pthread_t));

    /* Timer thread that wakes every function when needed. */
    pthread_create(&thread[0], NULL, eventTimer, NULL); 
    /* Wakes every 10 sec and returns a mac address. */
    pthread_create(&thread[1], NULL, BTnearMe, queues);
    /* Performs a covid test when woken. */
    pthread_create(&thread[2], NULL, testCOVID, NULL);
    /* Clears all close contacts. */
    pthread_create(&thread[3], NULL, clearCloseContacts, queues);
    /* Uploads the close contacts to binary file after positive covid test. */
    pthread_create(&thread[4], NULL, uploadContacts, queues);
    
    for(int i = 0; i < numWorkers; i++) pthread_join (thread[i], NULL); 
   
}