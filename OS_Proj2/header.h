/*
 * header.h
 *
 *  Created on: Oct 22, 2012
 *      Author: Tom Trask
 *     Purpose: Outline the functions used in p2.c
 * Description: Each function is used (for better or worse)
 * 				in the OS project 2.  Above each function
 * 				you will find a description of the purpose
 * 				of the function.  For more information about
 * 				each function, please look into the code.
 *
 * 				I dare you.
 */

//used to load the table in the file specified in
//arg1 into the STORE MANAGER for processing.
int LoadTable();

//forks the 2 child processes for processing
int ForkProcess();

//prints out the current status of the process
//specified by the pid
void ShowCurrentStatus(int pid);

//kills the process specified in the argument.
int KillProcess(int);

//allocates memory for the table used in the
//STORE MANAGER
int AllocateMemory();

//forks teh store manager process and
//does the things that the store manager
//should do.
pid_t ForkStoreManager();

//gets a message from the specified readpipe
//and sends a response through the specified
//writepipe.
int GetThreadedMessages(int pid, int readPipe[], int writePipe[]);

//Processes message sent through pipes 1 and 2
//to the STORE MANAGER
char* ProcessMessage(char[]);

//removes a newline character at the end of the specified string
//if there is one.
char* RemoveNewline(char*);

//Reads data from the specified pipe.  message is returned via the
//supplied msg.
int ReadFromPipe(int pid, int proc_id, int pipe[], char msg[]);

//writes data to the specified pipe.
int WriteToPipe(int pid, int proc_id, int pipe[], char* msg);

//updates the table row specified by the id to the supplied value.
int TABLE_UPDATE(char* id, int val);

//tries to update the table row specified by the id to the supplied .
int TABLE_READ(char* id, int* val);

//opens the log file for appending.
FILE* OpenLogFile();

//writes a message to the log file using the supplied arguments
int WriteToLogFile(FILE* fp, int pid, int proc_id, char* msg,
		short SendRecieved);

//closes the log file.
int CloseLogFile(FILE* fp);

//parses the cli arguments that specify the table to be loaded
//into the STORE MANAGER, the log file name, and the 2 transaction
//files to be used to update said table.
void SetCLIValues(char *arg, int i);

//gets input from the user.
//loops until the user decides to quit.
void GetInputFromUser();


//checks to see if the file specified exists.
//used to make sure that the CLI arguments are valid.
int file_exists(const char* fileName);


