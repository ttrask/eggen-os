/*
 * StoreMan.h
 *
 *  Created on: Oct 22, 2012
 *      Author: tom
 */

#ifndef STOREMAN_H_
#define STOREMAN_H_
#endif /* STOREMAN_H_ */

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
void ShowCurrentStatus(int);
pid_t ForkStoreManager();

void GetInputFromUser();

void SetCLIValues(char *arg, int i);


