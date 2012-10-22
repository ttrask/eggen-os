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
#include <fcntl.h>
#include <sys/shm.h>
#include <string.h>

int LoadTable();
pid_t ForkProcess();
int KillProcess(pid_t);
int AllocateSharedMemory();
int DeallocateSharedMemory();
int ReadFromPipe(int, int[]);
char* RemoveNewline(char*);
char* AddNewline(char[]);


char* exitCmd = "exit\n";
const int SIZE = 20;
int _tblSize = 0;

typedef struct {
	char* id;
	int value;
} TABLE;

TABLE* tbl;

int sharedMemId;
TABLE* sharedMemPtr;

pid_t p1, p2;
int pipe1[2], pipe2[2], nbytes;
int pipe1Done = 0, pipe2Done = 0;

int main() {

	pipe(pipe1);

	pipe(pipe2);

	//allocate shared memory
	AllocateSharedMemory();

	//load file data into store manager from INIT.DAT
	LoadTable();

	//fork 2 processes
	ForkProcess(p1, "TRANS1.txt", pipe1);
	ForkProcess(p2, "TRANS2.txt", pipe2);

	int quit = 0;
	char userInput;

	while (quit == 0) {

		if (pipe1Done == 0)
			ReadFromPipe(1, pipe1);

		if (pipe2Done == 0)
			ReadFromPipe(2, pipe2);

		usleep(100);
	}

	//set up signals for each process

	//read input from user until exit

	//deallocates shared memory
	DeallocateSharedMemory();

	KillProcess(p1);
	//KillProcess(p2);

	return 0;
}

int ReadFromPipe(int pid, int pipe[]) {

	char readBuffer[80];

	read(pipe[0], readBuffer, sizeof(readBuffer));

	if (strlen(readBuffer) > 0) {

		if (strcmp(readBuffer, exitCmd) == 0) {
			switch (pid) {
			case 1:
				pipe1Done = 1;
				break;
			case 2:
				pipe2Done = 1;
				break;
			}
		}

		//char* command = AddNewline(readBuffer);

		printf("Received String from process %d: %s", pid, readBuffer);

//		if (readBuffer[sizeof(readBuffer) - 1] != '\n')
//			printf("\n");

		ProcessMessage(readBuffer);
	}

	return 0;
}

int ProcessMessage(char* msg) {

	char cmd = strtok(msg, " ")[0];
	char* id = strtok(NULL, " ");

	if (id != NULL && strlen(id) > 0) {
		id = RemoveNewline(id);

		int val;

		switch (cmd) {
		case 'R':
			TABLE_READ(id, &val);
			break;
		case 'U':
			val = atoi(strtok(NULL, " "));
			TABLE_UPDATE(id, val);
			break;
		}
	} else {
		//printf("Error processing the command: %s\n -----No ID Provided-----\n",				msg);
	}

}

char* RemoveNewline(char* s) {

	if (strlen(s) > 0) {
		if (s[strlen(s) - 1] == '\n') {
			s[strlen(s) - 1] = '\0';
		}
	}
	return s;
}

char* AddNewline(char s[]) {

	char* to;

	if (strlen(s) > 0) {
		if (s[strlen(s) - 1] != '\n') {
			char* to = (char*) malloc(sizeof(s) + 1);
			strcpy(to, s);
			to[strlen(to) - 1] = '\n';
		}
	} else {
		char* to = (char*) malloc(sizeof(s));
		strcpy(to, s);
	}

	return to;
}

int TABLE_READ(char* id, int* val) {
	int i = 0;
	for (i = 0; i < _tblSize; i++) {
		if (strcmp(tbl[i].id, id) == 0) {
			val = tbl[i].value;
			printf("tbl[%s] = %d\n", id, val);
			return 1;
		}
	}
	return -1;
}

int TABLE_UPDATE(char* id, int val) {
	printf("Attempting to Update Table ID %s to value %d\n", id, val);
	int i = 0;
	for (i = 0; i < _tblSize; i++) {
		if (strcmp(tbl[i].id, id) == 0) {
			tbl[i].value = val;
			printf("tbl[%s] = %d\n", tbl[i].id,tbl[i].value );
			return 1;
		}
	}
	return -1;
}

pid_t ForkProcess(pid_t pid, char* transFileName, int pipe[]) {

	if ((pid = fork()) == 0) {

		//close read end of pipe for child process
		close(pipe[0]);

		char* filename = transFileName;
		char input[100];

		FILE *fp;

		fp = fopen(filename, "r");

		while (fgets(input, 100, fp) != NULL) {
			if (strlen(input) > 0) {
				char* str = input;

				write(pipe[1], str, strlen(str) + 1);
				sleep(1);
				input[0] = '\0';
			}
		}

		write(pipe[1], exitCmd, strlen(exitCmd) + 1);

		//printf("exiting child process %d\n", pid);

		close(pipe[1]);

		fclose(fp);

		exit(1);
		return 0;
	} else {

		//close write end of pipe for parent process
		close(pipe[1]);

		int quitLoop = 0;

		//loops until ReadFromPipe is done;

	}

}

int KillProcess(pid_t pid) {
	kill(pid, SIGKILL);

	return 0;
}

int AllocateSharedMemory() {

	if ((tbl = malloc(SIZE * sizeof(TABLE*))) == -1) {
		//implement fallout for shmget failure
	}

//gets a pointer to the allocated memory

	return 0;

}

int DeallocateSharedMemory() {
//deallocates shared memory
//	shmdt(sharedMemPtr);
//	shmctl(sharedMemId, IPC_RMID, NULL);

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

	_tblSize = index;

	int i = 0;

	for (i = 0; i < index; i++) {
		printf("Input: %s %d\n", tbl[i].id, tbl[i].value);
	}

	fclose(fp);

	return 0;
}
