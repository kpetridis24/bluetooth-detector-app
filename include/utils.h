#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define QUEUESIZE 1500
#define NUM_ADDRESSES 25 //300
#define NUM_CHARS 12
#define NON_CLOSE_CONTACT_DUR 30.0  //1200.0 
#define CLOSE_CONTACT_DUR 70.0   //10800.0
#define BT_RATE 2.0 //10.0
#define COV_RATE 8.0   //2000.0   //1800, 30min
#define TERMINATION_TIME 150.0  //25200.0



bool isSameCode(char *code1, char *code2){

    for(int i = 0; i < NUM_CHARS; i++)
        if(code1[i] != code2[i]) return false;
    return true;
}


char **randomHex(int length, int num){

    char str[] = "0123456789ABCDEF";
    char **hexNums = (char **)malloc(num * sizeof(char*));
    for(int i=0; i < num; i++)hexNums[i] = (char *)malloc(length * sizeof(char));

    /* Seed number for rand() */
    srand((unsigned int) time(0) + getpid());

    for(int i = 0; i < num; i++){
        for(int j = 0; j < length; j++){
            hexNums[i][j] = str[rand() % 16];
            srand(rand());
        }
    }
    return hexNums;
}


struct timeval tic(){
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}


double toc(struct timeval begin){
    
    struct timeval end;
    gettimeofday(&end, NULL);
    double stime = ((double)(end.tv_sec-begin.tv_sec)*1000)+
                            ((double)(end.tv_usec-begin.tv_usec)/1000);
    stime /= 1000;
    return stime;
}


#endif