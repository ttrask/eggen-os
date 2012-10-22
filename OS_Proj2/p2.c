/*
 * main.c
 *
 *  Created on: Oct 22, 2012
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
#include <string.h>

int LoadTable();
pid_t ForkProcess();
int KillProcess(pid_t);
int AllocateSharedMemory();
int DeallocateSharedMemory();

const int SIZE = 20;

typedef struct {
	char* id;
	int value;
} TABLE;

TABLE* tbl;

int sharedMemId;
TABLE* sharedMemPtr;

int main() {

	pid_t p1, p2;

	//allocate shared memory
	AllocateSharedMemory();

	//load file data into store manager from INIT.DAT
	LoadTable();

	//fork 2 processes
	p1 = ForkProcess(1);
	p2 = ForkProcess(2);

	//set up signals for each process

	//read input from user until exit

	//deallocates shared memory
	DeallocateSharedMemory();

	KillProcess(p1);
	KillProcess(p2);

	return 0;
}

pid_t ForkProcess(int id) {
	pid_t procID;
	int status;
	if ((procID = fork()) == 0) {

		printf("Created Child process with PID:%d", procID);
		while (1 == 1) {
			switch (id) {
			case 1:

				break;
			case 2:
				sleep(1);
				break;

			default:
				while (1 == 1) {
					printf("invalid thread specified");
				}
				break;
			}
		}
		exit(1);
		return 0;
	} else {
//		if ((procID = wait(&status)) == -1) {
//			perror("error waiting for children to complete.\n");
//
//		} else {
//			printf("Child process ended normally.n");
//		}

		return procID;
	}

}

int KillProcess(pid_t pid) {
	kill(pid, SIGKILL);

	return 0;
}

int AllocateSharedMemory() {

	if ((sharedMemId = shmget(IPC_PRIVATE, SIZE * sizeof(TABLE*), 0660))
			== -1) {
		//implement fallout for shmget failure
	}

//gets a pointer to the allocated memory
	if ((sharedMemPtr = (TABLE *) shmat(sharedMemId, (void *) 0, 0))
			== (void *) -1) {
		//implement fallout for shmgat failure
	}

	tbl = sharedMemPtr;

	return 0;

}

int DeallocateSharedMemory() {
//deallocates shared memory
	shmdt(sharedMemPtr);
	shmctl(sharedMemId, IPC_RMID, NULL);

	return 0;

}

int LoadTable() {

	char* filename = "INIT.DAT";

	FILE *fp;

	fp = fopen(filename, "r");

	char input[100];

	int index = 0;

	while (fgets(input, 100, fp) != 0) {

		char* id = "";
		int value = 0;

		//get id  & value from input line
		id = strtok(input, " ");
		value = atoi(strtok(NULL, "\n"));

		//allocate memory space for the ID part of the table row
		tbl[index].id = (char*) calloc(strlen(id) + 1, sizeof(char));

		//copy row information into new table row.
		strcpy(tbl[index].id, id);
		tbl[index].value = value;

		index++;
	}

	int i = 0;

	for (i = 0; i < index; i++) {
		printf("Input: %s %d\n", tbl[i].id, tbl[i].value);
	}

	fclose(fp);

	return 0;
}
