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
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <string.h>
#include "header.h"

#ifndef NULL
#define NULL   ((void *) 0)
#endif

FILE* logFilePointer;

char* exitCmd = "exit\n";

char* initFileName = "";
char* logFileName = "";
char* trans1FileName = "";
char* trans2FileName = "";
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

int main(int argc, char *argv[]) {

	//assume first argument is the application name
	if (argc < 5) {
		printf("Error: Invalid number of command line arguments\n");
		printf("Program expected the following cli arguments\n");
		printf("./p2 init.file log.file, trans1.file, trans2.file\n");
		exit(0);
	}

	int i = 0;

	//Gets CLI Values.  Tanks if values not provided.
	for (i = 1; i < argc; i++) {
		//printf("CLI Value:%s\n", argv[i]);

		SetCLIValues(argv[i], i);
	}

	//open all 4 pipes
	pipe(pipe1);
	pipe(pipe2);
	pipe(pipe3);
	pipe(pipe4);

	//fork store manager
	pid_t sm_pid = ForkStoreManager();
	//printf("Forked Store Manager\n");

	//wait a sec for the store manager to load the input file.
	//this is less to do with the store manager doing what it's
	//supposed to do and more to prevent the forked processes
	//from executing
	//sleep(1);

	pid1 = ForkProcess(1, trans1FileName, pipe1, pipe3);
	//printf("Forked Process 1\n");
	pid2 = ForkProcess(2, trans2FileName, pipe2, pipe4);
	//printf("Forked Process 2\n");

	GetInputFromUser();

	//deallocates shared memory

	KillProcess(sm_pid);
	KillProcess(pid1);
	KillProcess(pid2);

	return 0;
}

void GetInputFromUser() {

	char userInput[80];

	while (1 == 1) {

		printf(
				"Press '1' for status of process 1, '2' for status from process 2, or '3' to quit:\n");

		scanf("%s", userInput);

		//userInput = RemoveNewline(userInput);

		if (strcmp(userInput, "1") == 0) {
			kill(pid1, SIGUSR1);

		} else if (strcmp(userInput, "2") == 0) {
			kill(pid2, SIGUSR2);

		} else if (strcmp(userInput, "3") == 0) {
			printf("Exiting Application\n");
			return;
		} else {
			printf("Invalid input!\n");
		}

		usleep(200);

	}

}

pid_t ForkStoreManager() {

	pid_t sm_pid;

	int quit = 0;

	switch ((sm_pid = fork())) {
	case 0:

		logFilePointer = OpenLogFile();

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

//		printf("Received String from process %d: %s\n", pid,
//				RemoveNewline(readBuffer));

		char* msg = ProcessMessage(readBuffer);

		//sends response to write pipe.
		WriteToPipe(0, 0, writePipe, msg);
	}

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

	//printf("Received Message '%s' to Process Manger\n", msg);

	int id = atoi(strtok(msg, " "));
	int pid = atoi(strtok(NULL, " "));
	char cmd = strtok(NULL, " ")[0];
	char* key = strtok(NULL, " ");

	int success = 0;
	int val = 0;
	int* valPtr = &val;
	char* valStr = "";

	if (key != NULL && strlen(key) > 0) {
		key = RemoveNewline(key);

		switch (cmd) {
		case 'R':

			success = TABLE_READ(key, valPtr);
			break;
		case 'U':
			valStr = strtok(NULL, " ");

			if (strlen(valStr) > 0) {
				if (isNumeric(valStr)) {
					val = atoi(valStr);
					success = TABLE_UPDATE(key, val);
				} else
					success = -1;
			} else {
				success = -1;
			}
			break;

		default:
			success = -1;
			break;
		}
	} else {
		success = -1;
	}

	sprintf(msg, "%d %d %c %s", id, 0, cmd, key);

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
			//printf("tbl[%s] = %d\n", id, *val);
			return 0;
		}
	}
	return -1;
}

int TABLE_UPDATE(char* id, int val) {
//printf("Attempting to Update Table ID %s to value %d\n", id, val);
	int i = 0;
	for (i = 0; i < _tblSize; i++) {
		if (strcmp(tbl[i].id, id) == 0) {
			tbl[i].value = val;
			//printf("tbl[%s] = %d\n", tbl[i].id, tbl[i].value);
			return 0;
		}
	}
	return -1;
}

int failedCommands = 0;
int totalCommands = 0;

int ForkProcess(int id, char* transFileName, int writePipe[],
		int responsePipe[]) {

	pid_t p;

	if ((p = fork()) == 0) {

		//close read end of pipe for child process

		p = getpid();

		close(writePipe[0]);
		close(responsePipe[1]);

		signal(SIGUSR1, ShowCurrentStatus);
		signal(SIGUSR2, ShowCurrentStatus);

		char* filename = transFileName;
		char input[100] = "";

		logFilePointer = OpenLogFile();

		FILE *fp;

		fp = fopen(filename, "r");

		while (fgets(input, 100, fp) != NULL) {
			if (strlen(input) > 0) {
				char* str = input;
				char buf[80] = "";
//				printf("attempting to send %s through pipe.\n",
//						RemoveNewline(input));
				sprintf(buf, "%d %d %s", id, p, str);
//				printf("sending %s\n", RemoveNewline(buf));
				WriteToPipe(id, p, writePipe, buf);

				sleep(1);
				char readBuffer[80];

				ReadFromPipe(id, p, responsePipe, readBuffer);

				if (strcmp(readBuffer + strlen(readBuffer) - strlen("FAILED"),
						"FAILED") == 0)
					failedCommands++;

				totalCommands++;

				input[0] = '\0';
			}
		}

		//write(writePipe[1], exitCmd, strlen(exitCmd) + 1);

		//printf("exiting child process %d\n", pid);

		close(writePipe[1]);
		close(responsePipe[1]);

		fclose(fp);

		while (1 == 1) {
			sleep(1);
		}

		return 0;
	} else {

		//close write end of pipe for parent process
		close(writePipe[1]);
		close(responsePipe[0]);

		//loops until ReadFromPipe is done;
		return p;
	}

}

void ShowCurrentStatus(int signum) {

//	printf("Reached Signal\n");
	int id = 0;

	switch (signum) {
	case SIGUSR1:
		id = 1;
		break;
	case SIGUSR2:
		id = 2;
		break;
	}

	printf("Process %d status %d/%d\n", id, (totalCommands - failedCommands),
			totalCommands);

	return;
}

FILE* OpenLogFile() {

	FILE *fp;
	fp = fopen(logFileName, "a+");
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

	CloseLogFile(fp);

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

int LoadTable() {

	char* filename = initFileName;

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
//		printf("Input: %s %d\n", tbl[i].id, tbl[i].value);
	}

	fclose(fp);

	return 0;
}

void SetCLIValues(char *arg, int i) {

	//check if the specified file exists
	//don't check if the log file exists.
	if (i != 2 && !file_exists(arg)) {
		printf("File %s does not exist!\n", arg);
		exit(0);
	}

	switch (i) {
	case 1:
		initFileName = arg;
		printf("Using %s as Init Store Manager File\n", initFileName);

		break;
	case 2:
		logFileName = arg;
		printf("Using %s as Log File\n", logFileName);
		break;
	case 3:
		trans1FileName = arg;
		printf("Using %s for Process 1 Transactions\n", trans1FileName);
		break;
	case 4:
		trans2FileName = arg;
		printf("Using %s for Process 2 Transactions\n", trans2FileName);
		break;
	}

	return;
}

int isNumeric(const char * s) {
	if (s == NULL || *s == '\0' || isspace(*s))
		return 0;
	char * p;
	strtod(s, &p);
	return *p == '\0';
}

int file_exists(const char * filename) {
	FILE* file;
	if (file = fopen(filename, "r")) {
		fclose(file);
		return 1;
	}

	return 0;
}
