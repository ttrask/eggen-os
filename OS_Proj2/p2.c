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
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <string.h>

int LoadTable();
int ForkProcess();
int KillProcess(int);
int AllocateMemory();
int DeallocateSharedMemory();
char* ProcessMessage(char[]);
int GetThreadedMessages(int, int[], int[]);
char* RemoveNewline(char*);
FILE* OpenLogFile();
int ReadFromPipe(int pid, int proc_id, int pipe[], char msg[]);
int WriteToPipe(int pid, int proc_id, int pipe[], char* msg);
int TABLE_UPDATE(char* id, int val);
int TABLE_READ(char* id, int* val);
int WriteToLogFile(FILE* fp, int pid, int proc_id, char* msg,
		short SendRecieved);
int CloseLogFile(FILE* fp);

#ifndef NULL
#define NULL   ((void *) 0)
#endif

FILE* logFilePointer;
char* exitCmd = "exit\n";
char* logFileName = "LOG.DAT";
char* trans1FileName = "TRANS1.txt";
char* trans2FileName = "TRANS2.txt";
const int SIZE = 20;
int _tblSize = 0;

typedef struct {
	char* id;
	int value;
} TABLE;

TABLE* tbl;

int sharedMemId;
TABLE* sharedMemPtr;

pid_t pid1, pid2;
int p1, p2, storeManager;
int pipe1[2], pipe2[2], pipe3[2], pipe4[2], nbytes;
int pipe1Done = 0, pipe2Done = 0;

int main() {

	pipe(pipe1);
	pipe(pipe2);
	pipe(pipe3);
	pipe(pipe4);

	//fork 2 processes

	pid_t sm_pid = ForkStoreManager();
	sleep(1);
	pid_t* p1p = &pid1, p2p = &pid2;

	//logFilePointer = OpenLogFile();

	ForkProcess(1, trans1FileName, pipe1, pipe3);
	ForkProcess(2, trans2FileName, pipe2, pipe4);

	//CreateStoreManager();
	int quit = 0;
	char userInput;

	//load file data into store manager from INIT.DAT

	//set up signals for each process

	//read input from user until exit
	sleep(20);
	//deallocates shared memory
	DeallocateSharedMemory();

	KillProcess(sm_pid);
	KillProcess(p1);
	KillProcess(p2);

	CloseLogFile(logFilePointer);

	return 0;
}

int ForkStoreManager() {

	pid_t sm_pid;

	int quit = 0;

	switch ((sm_pid = fork())) {
	case 0:

		//allocate shared memory
		AllocateMemory();

		LoadTable();

		while (quit == 0) {

			if (pipe1Done == 0)
				GetThreadedMessages(1, pipe1, pipe3);

			if (pipe2Done == 0)
				GetThreadedMessages(2, pipe2, pipe4);

			if (pipe1Done != 0 && pipe2Done != 0) {
				printf("quitting app\n");
				quit = 1;
			}

			usleep(100);
		}

		exit(1);

		break;

	}
	return sm_pid;

}
int GetThreadedMessages(int pid, int readPipe[], int writePipe[]) {

	char readBuffer[80];

	ReadFromPipe(0, 0, readPipe, readBuffer);

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

		printf("Received String from process %d: %s\n", pid,
				RemoveNewline(readBuffer));
		char* msg = ProcessMessage(readBuffer);

		//sends response to write pipe.
		WriteToPipe(0, 0, writePipe, msg);
	}

	//free(readBuffer);

	return 0;
}

int ReadFromPipe(int pid, int proc_id, int pipe[], char msg[]) {

	char readBuffer[80] = "";

	read(pipe[0], readBuffer, sizeof(readBuffer));

	strcpy(msg, readBuffer);

	WriteToLogFile(logFilePointer, pid, proc_id, msg, 2);

	return 1;

}

int WriteToPipe(int pid, int proc_id, int pipe[], char* msg) {

	write(pipe[1], msg, 80);

	WriteToLogFile(logFilePointer, pid, proc_id, msg, 1);

	return 1;

}

char* ProcessMessage(char msg[]) {

	int id = atoi(strtok(msg, " "));
	int pid = atoi(strtok(NULL, " "));
	char cmd = strtok(NULL, " ")[0];
	char* key = strtok(NULL, " ");

	int success = 0;
	int val = 0;
	int* valPtr = &val;

	if (key != NULL && strlen(key) > 0) {
		key = RemoveNewline(key);

		switch (cmd) {
		case 'R':

			success = TABLE_READ(key, valPtr);
			break;
		case 'U':
			val = atoi(strtok(NULL, " "));
			success = TABLE_UPDATE(key, val);
			break;
		default:
			val = -1;
			break;
		}
	} else {
		//printf("Error processing the command: %s\n -----No ID Provided-----\n",				msg);
	}

	if (success == -1)
		return strcat(msg, " FAILED");
	else {

		char buf[10] = "";

		sprintf(buf, " %d", val);

		return strcat(msg, buf);
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

int TABLE_READ(char* id, int* val) {
	int i = 0;
	for (i = 0; i < _tblSize; i++) {
		if (strcmp(tbl[i].id, id) == 0) {
			*val = tbl[i].value;
			printf("tbl[%s] = %d\n", id, *val);
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
			printf("tbl[%s] = %d\n", tbl[i].id, tbl[i].value);
			return 1;
		}
	}
	return -1;
}

int ForkProcess(int id, char* transFileName, int writePipe[],
		int responsePipe[]) {

	pid_t p;

	if ((p = fork()) == 0) {

		//close read end of pipe for child process

		p = getpid();

		close(writePipe[0]);
		close(responsePipe[1]);

		char* filename = transFileName;
		char input[100] = "";

		FILE *fp;

		fp = fopen(filename, "r");

		while (fgets(input, 100, fp) != NULL) {
			if (strlen(input) > 0) {
				char* str = input;
				char buf[80] = "";
				printf("attempting to send %s through pipe.", input);
				sprintf(buf, "%d %d %s", id, p, str);
				printf("sending %s", buf);
				WriteToPipe(id, p, writePipe, buf);

				sleep(1);
				char readBuffer[80];

				ReadFromPipe(id, p, responsePipe, readBuffer);

				input[0] = '\0';
			}
		}

		write(writePipe[1], exitCmd, strlen(exitCmd) + 1);

		//printf("exiting child process %d\n", pid);

		close(writePipe[1]);
		close(responsePipe[1]);

		fclose(fp);

		while (1 == 1) {
			sleep(1);
		}

		exit(1);
		return 0;
	} else {

		switch (id) {
		case 1:
			pid1 = p;
			break;
		case 2:
			pid2 = p;
			break;
		}

		//close write end of pipe for parent process
		close(writePipe[1]);
		close(responsePipe[0]);

		//loops until ReadFromPipe is done;

	}

}

FILE* OpenLogFile() {

	FILE *fp;
	fp = fopen(logFileName, "a");
	return fp;
}

int WriteToLogFile(FILE* fp, int pid, int proc_id, char* msg, short sendRecieve) {

	fp = OpenLogFile();

	time_t clock = time(NULL);

	char procName[20];

	switch (pid) {
	case 0:
		strcpy(procName, "STORE MANAGER");
		break;
	case 1:
		strcpy(procName, "Process 1");
		break;
	case 2:
		strcpy(procName, "Process 2");
		break;

	}

//remove newline character from timestamp
	time(&clock);

	char timef[60];
	strcpy(timef, ctime(&clock));

	timef[strlen(timef) - 1] = '\0';

	fprintf(fp, "%s at time %s %s command %s \n", procName, timef,
			(sendRecieve == 1 ? "sent" : "received"), RemoveNewline(msg));

	fclose(fp);
	return 1;
}

int CloseLogFile(FILE* fp) {

	fclose(fp);
	return 1;
}

int KillProcess(int pid) {
	kill(pid, SIGKILL);

	return 0;
}

int AllocateMemory() {

	if ((tbl = malloc(SIZE * sizeof(TABLE*))) == -1) {

	}

	return 0;

}

int DeallocateSharedMemory() {

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
