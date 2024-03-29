/*
 * os1.c
 *
 *  Created on: Sep 26, 2012
 *      Author: tom
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>

int producerCount = 0;
int consumerCount = 0;
int semCount = 0;
//wait time of 10ms
int waitTime = 10;
const int queueSize = 10;
int fullCount = 0;
int emptyCount = 10;
int itemsProduced = 0;
int totalItems = 0;
int itemCount = 0;
int sharedMemId;
int* sharedMemPtr;
//Function Prototypes
void SetCLIValues(char*, int);
void LockSemaphore(int id, int i);
void UnlockSemaphore(int id, int i);
int isNumeric(char*);

enum {
	QUEUE_EMPTY, QUEUE_FULL, QUEUE_ACCESS
};

int main(int argc, char *argv[]) {

	int* queuePtr;

	int producersDone = 0, consumersDone = 0;
	int i = 0, j = 0;
	int status;
	int itemsConsumed = 0;
	int* itemsConsumedPtr;

	short sems[3];
	int semid;

	key_t key = 4590;

	int fullSem = 0, emptySem = 0, accessSem = 0;

	struct sembuf arg;

	//assume first argument is the application name
	if (argc < 4) {
		printf("Error: Invalid input");
		exit(0);
	}

	//Gets CLI Values.  Tanks if values not provided.
	for (i = 1; i < argc; i++) {
		//printf("CLI Value:%s\n", argv[i]);

		SetCLIValues(argv[i], i);
	}

	//indicates the total number of items to be created.

	totalItems = producerCount * itemCount;

	printf("Using %d producers to produce %d items\n", producerCount,
			totalItems);

	//gets the requisite set of allocated memory for producers and consumers
	//to pump data into/out of.
	if ((sharedMemId = shmget(IPC_PRIVATE, (queueSize + 2) * sizeof(int), 0660))
			== -1) {
		//implement fallout for shmget failure
	}
	printf("allocating %d ints at shared memory id %d\n", queueSize,
			sharedMemId);

	//gets a pointer to the allocated memory
	if ((sharedMemPtr = (int *) shmat(sharedMemId, (void *) 0, 0))
			== (void *) -1) {
		//implement fallout for shmgat failure
	}

	//sets the queue to point to the shared memory
	queuePtr = sharedMemPtr;

	//initialize queue to -1;
	for (i = 0; i < queueSize; i++) {
		queuePtr[i] = -1;
	}

	//total items consumed
	queuePtr[queueSize] = 0;
	//total items produced
	queuePtr[queueSize + 1] = 0;

	sems[QUEUE_FULL] = 0;
	sems[QUEUE_EMPTY] = 1;
	sems[QUEUE_ACCESS] = 2;

	//Allocate Semaphore.  Gets Semaphore ID
	if ((semid = semget(key, 3, 0600 | IPC_CREAT)) == -1) {
		printf("error getting semaphore\n");
		exit(1);
	} else {
		printf("Allocated 3 Semaphores at semaphore ID:%d\n", semid);
	}

	semctl(semid, 0, SETALL, sems);

	fullSem = sems[QUEUE_FULL];
	emptySem = sems[QUEUE_EMPTY];
	accessSem = sems[QUEUE_ACCESS];

	UnlockSemaphore(semid, emptySem);
	UnlockSemaphore(semid, fullSem);
	UnlockSemaphore(semid, accessSem);

	LockSemaphore(semid, emptySem);
	//generates an array of consumers and producers.
	pid_t prodIds[producerCount];
	pid_t consIds[consumerCount];

	//for producer processes

	for (i = 0; i < producerCount; i++) {
		switch (prodIds[i] = fork()) {
		case -1:

			printf("Error forking producer\n");
			break;
		case 0:

			//printf("producer created with id=%d, pid=%d\n", prodIds[i],
//					getpid());
//			printf("Queue Full Semaphore: %d\n",					semctl(semid, fullSem, GETVAL, arg.sem_num));
			for (i = 0; i < itemCount; i++) {
				int hasAddedToQueue = 0;

				while (hasAddedToQueue == 0) {
					//if queue isn't full, addd something to queue;

					//put something into the queue
					//loop through queue to find an empty spot
					//as indicated by the value = -1

					//check to see if the queue is available
					if (semctl(semid, fullSem, GETVAL, arg.sem_num) > 0) {

						while (semctl(semid, accessSem, GETVAL, arg.sem_num)
								<= 0) {
							usleep(waitTime);
						}

						//lock semaphore
						LockSemaphore(semid, accessSem);
						//if it is, do the do.
						for (j = 0; j < queueSize; j++) {
							if (queuePtr[j] == -1) {
								if (queuePtr[queueSize + 1] >= totalItems) {
//									printf("Completed Producing %d items",
//											totalItems);
//									printf("exiting producer");
									exit(0);
								}

								//do your stuff
								queuePtr[j] = itemsProduced;
								queuePtr[queueSize + 1]++;


								printf(
										"%d\t%d\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t\n",
										getpid(),
										0,
										1,
										semctl(semid, emptySem, GETVAL, arg.sem_num),
										semctl(semid, fullSem, GETVAL, arg.sem_num),
										semctl(semid, accessSem, GETVAL, arg.sem_num),
										"Produce", queuePtr[queueSize+1], queuePtr[queueSize]);
								//unlock it

								itemsProduced++;
								hasAddedToQueue = 1;
								fullCount++;
								emptyCount--;
								j = queueSize;
							}

							if (semctl(semid, emptySem, GETVAL, arg.sem_num)
									<= 0) {
								UnlockSemaphore(semid, emptySem);
							}
						}
						UnlockSemaphore(semid, accessSem);
						//if the queue is full, lock the queue
						if (fullCount == queueSize || hasAddedToQueue == 0) {
							hasAddedToQueue = 1;
							//LockSemaphore(semid, fullSem);
						}
					} else {

						usleep(waitTime);
					}
				}
			}

			//printf("Exiting Producer Process %d\n", getpid());
			exit(EXIT_SUCCESS);
			break;

		case 1:
			printf("producer parent process ID %d\n", getpid());

			//waits around for child processes to get done.
			if ((prodIds[i] = wait(&status)) == -1) {
				perror("error waiting for children to complete.\n");

			} else {
				printf("Child process ended normally.n");
				producersDone = 1;
			}

			break;
		}
	}

	for (i = 0; i < consumerCount; i++) {
		switch (consIds[i] = fork()) {
		case -1:

			printf("Error forking consumer\n");
			break;
		case 0:

			printf("consumer created with id=%d, pid=%d, parent pid=%d\n",
					consIds[i], getpid(), getppid());

			printf("Queue Empty: %d\n",
					semctl(semid, emptySem, GETVAL, arg.sem_num));
			//consume until total number of items have been consumed
			while (queuePtr[queueSize] < totalItems) {

				//if queue is empty, wait around until something comes into it.
				if (semctl(semid, emptySem, GETVAL, arg.sem_num) > 0) {
					//consume item.
					int foundValue = 0;

					while (semctl(semid, accessSem, GETVAL, arg.sem_num) <= 0) {
						usleep(waitTime);
					}
					LockSemaphore(semid, accessSem);

					for (j = 0; j < queueSize; j++) {
						if (queuePtr[j] != -1) {
							//print out consumer

							queuePtr[queueSize]++;


							printf(
									"%d\t%d\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t\n",
									getpid(),
									0,
									1,
									semctl(semid, emptySem, GETVAL, arg.sem_num),
									semctl(semid, fullSem, GETVAL, arg.sem_num),
									semctl(semid, accessSem, GETVAL, arg.sem_num),
									"Consume", queuePtr[queueSize+1], queuePtr[queueSize]);
							queuePtr[j] = -1;
							//increments total items consumed
							emptyCount++;
							foundValue = 1;

							if (semctl(semid, fullSem, GETVAL, arg.sem_num)
									<= 0) {
								UnlockSemaphore(semid, fullSem);
							}

							if (queuePtr[queueSize] >= totalItems) {
								if (queuePtr[queueSize] == totalItems) {
									printf(
											"All consumption has been completed.\n");
								}
								//printf("Exiting Consumer Thread %d", getpid());
								exit(0);
								//all consumption is done.
							}

							j = queueSize;
						}
					}
					UnlockSemaphore(semid, accessSem);

					if (fullCount <= 0 || foundValue == 0) {
						//LockSemaphore(semid, emptySem);

					}
				} else {
					usleep(waitTime);
				}
			}

			printf("Exiting Consumer Process %d\n", getpid());
			break;

		case 1:

			printf("consumer parent process ID %d\n", getpid());

			//waits around for child processes to get done.
			if (waitpid(prodIds[i], &status, WUNTRACED) == -1) {
				perror("error waiting for children to complete.\n");
			} else if (WIFEXITED(status) != 0) {
				printf("Child process ended normally; status = %d.n",
						WEXITSTATUS(status));
				consumersDone = 1;
			}

			break;
		}
	}
//printf("Allocated %d semaphores with id: %d\n", queueSize, semid);

//	printf("press any key to deallocate semaphore\n" );
//	getchar();

//	for (i = 0; i < producerCount; i++) {
//		if (waitpid(prodIds[i], &status, 0) == -1)
//			printf("Error exiting child process");
//
//	}
//	for (i = 0; i < consumerCount; i++) {
//		if (waitpid(consIds[i], &status, 0) == -1)
//			printf("Error exiting child process");
//
//	}

	while (wait(&status) > 0)

//deallocate shared memory

		shmdt(sharedMemPtr);
//delete allocated semaphores
	semctl(semid, 0, IPC_RMID);

//printf("Deallocated Semaphore\n");

	return 0;
}

void SetCLIValues(char *arg, int i) {

	if (isNumeric(arg) != 0) {
		switch (i) {
		case 1:
			producerCount = atoi(arg);
			printf("Using %d producers\n", producerCount);
			break;
		case 2:
			consumerCount = atoi(arg);
			printf("Using %d consumers\n", consumerCount);
			break;
		case 3:
			itemCount = atoi(arg);
			printf("Using %d items\n", itemCount);
			break;
		}
	} else {
		switch (i) {
		case 1:
			printf("Error: Invalid Producer value");
			exit(0);
			break;
		case 2:
			printf("Error: Invalid consumer value");
			exit(0);
			break;
		case 3:
			printf("Error: Invalid item count value");
			exit(0);
			break;
		}

	}

	return;
}

void LockSemaphore(int id, int i) {

	//printf("Locking semaphore %d\n", i);

	if (semctl(id, i, GETVAL, NULL) == -1) {
		printf("Semaphore already locked!");
		return;
	}

	struct sembuf sb;
	sb.sem_num = i;
	sb.sem_op = -1;
	sb.sem_flg = SEM_UNDO;
	semop(id, &sb, 1);
}

void UnlockSemaphore(int id, int i) {
	//printf("Unlocking semaphore %d\n", i);

	struct sembuf sb;
	sb.sem_num = i;
	sb.sem_op = 1;
	sb.sem_flg = SEM_UNDO;
	semop(id, &sb, 1);
}

int isNumeric(char * s) {
	if (s == NULL || *s == '\0' || isspace(*s))
		return 0;
	char * p;
	strtod(s, &p);
	return *p == '\0';
}

