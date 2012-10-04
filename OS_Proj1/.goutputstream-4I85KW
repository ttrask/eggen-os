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
    struct sembuf arg;

    /* grab the semaphore set created by seminit.c: */
    if ((semid = semget(key, 3, 0)) == -1) {
        printf("error reporting semaphore value\n");
        exit(1);
    }
    printf("%5d %d %s\n",
        getpid(), semctl(semid, 0, GETVAL, arg.sem_num), "value");
    printf("%5d %d %s\n",
        getpid(), semctl(semid, 1, GETVAL, arg.sem_num), "value");
    printf("%5d %d %s\n",
        getpid(), semctl(semid, 2, GETVAL, arg.sem_num), "value");
//  printf("%5d %d %s\n",
//      getpid(), semctl(semid, 5, GETVAL, arg.sem_num), "value");

    return 0;
}
