/** 
   Program to create a semaphore, taken from 
   www.ecst.csuchico.edu/~beej/guide/ipc/semaphores.html

   Modified by Dr. Roger Eggen on 9/27/00 in an effort to
   correct some errors in the program.
  */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main(void)
{
    key_t key = 1234;
    int semid;
    struct sembuf sb = {0, -1, 0}; /* set to decrement */
    struct sembuf nb = {0, 1, 0};  /* set to increment */
    struct sembuf nc = {1, 1, 0};  /* set to increment */
    struct sembuf nd = {2, 9, 0};  /* set to increment */
    struct sembuf arg;

    /* grab the semaphore set created by seminit.c: */
    if ((semid = semget(key, 1, 0)) == -1) {
        printf("error incrementing semaphore \n");
        exit(1);
    }
//    semop(semid, &sb,1);   // this is a wait
//    semop(semid, &nc,1);   // this is a signal
    semop(semid, &nb,1);   // this is a signal
//    semop(semid, &nd,1);   // this is a signal

    return 0;
}
