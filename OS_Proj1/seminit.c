/** 
   Program to create a semaphore, taken from 
   www.ecst.csuchico.edu/~beej/guide/ipc/semaphores.html

   Modified by Dr. Roger Eggen on 9/27/00 in an effort to correct
   some errors in the program.
**/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main_seminit(void)
{
    key_t key = 1234;
    int semid;
/*     union semun arg; */
    struct sembuf arg;

    /* create a semaphore set with 3 semaphore: */
    if ((semid = semget(key, 3, 0600 | IPC_CREAT)) == -1) {
        printf("error in semget");
        exit(1);
    }

    /* initialize semaphore #0 to 1: */
    arg.sem_num = 1;
    if (semctl(semid, 0, SETVAL, arg.sem_num) == -1) { 
        printf("error in semctl");
        exit(1);
    }

    return 0;
}
