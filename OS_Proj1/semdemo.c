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

int main_semdemo(void) {
	key_t key = 1234;
	int semid;
	struct sembuf sb = { 0, -1, 0 }; /* set to decrement */
	struct sembuf nb = { 0, 1, 0 }; /* set to increment */
	struct sembuf arg;

	printf("The semaphore is decremented until it hits 0. \n");
	printf("When the semaphore to decrement again, the program is  \n");
	printf("hung waiting on the semaphore.                \n");
	printf("Comment out the second decrement and the program\n");
	printf("will run.                                       \n");

	/* grab the semaphore set created by seminit.c: */
	if ((semid = semget(key, 3, 0)) == -1) {
		printf("error getting semaphore\n");
		exit(1);
	}

	printf("%5d %d %s\n", getpid(), semctl(semid, 0, GETVAL, arg.sem_num),
			"ONE");

	printf("Press return to block: ");
	getchar();
	printf("Trying to block...\n");
	printf("%5d %d %s\n", getpid(), semctl(semid, 0, GETVAL, arg.sem_num),
			"TWO");
	/* decrements to 0 */
	if (semop(semid, &sb, 1) == -1) {
		printf("error decrementing semaphore \n");
		exit(1);
	}
	printf("%5d    %d     %s\n\n", getpid(),
			semctl(semid, 0, GETVAL, arg.sem_num), "THREE");

	/* tries to decrement to -1 but hangs since the semaphore is 0*/
	printf(" tries to decrement to -1 but hangs since the semaphore is 0\n");
	if (semop(semid, &sb, 1) == -1) {
		printf("error decrementing semaphore \n");
		exit(1);
	}
	printf("%5d    %d     %s\n\n", getpid(),
			semctl(semid, 0, GETVAL, arg.sem_num), "FOUR");

	printf("Locked if further access to decrement the semaphore.\n");
	printf("Press return to unblock: ");
	getchar();

	sb.sem_op = 1; /* free resource */
	if (semop(semid, &nb, 1) == -1) {
		printf("error incrementing semaphore \n");
		exit(1);
	}

	printf("Unblocked\n");
	printf("%5d    %d     %s\n", getpid(),
			semctl(semid, 0, GETVAL, arg.sem_num), "incr");

	return 0;
}
