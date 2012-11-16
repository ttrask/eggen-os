#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define MAXLINE 100
#define NAME_SIZE 50
#define SIZE 100

typedef struct table {
	char* id;
	int value;
	struct table *next;
} TABLE;
TABLE *first = NULL;
TABLE *last = NULL;

void store_manager(int, int, int, int, char*, char*, int);
void proc1(int, int, char*, int, char*);
void proc2(int, int, char*);
int table_update(char*, TABLE*, int);
int table_read(char*, TABLE* db);
void parent();
void my_handler(int);
void message(char*, char *, int, char*, char*, int, int, int);

FILE* f_ini;
FILE* f_proc1;
FILE* f_proc2;
FILE* f_log;
int exe1;
int exe2;
int suc1;
int suc2;
int count;
pid_t child_a;
pid_t child_b;
pid_t child_c;
int incoming;
int done;
char* log_file;

int main(int argc, char **argv) {
	int P1[2], P2[2], P3[2], P4[2];
	int p, entry;

	pipe(P1);
	pipe(P2);
	pipe(P3);
	pipe(P4);

	child_a = fork();

	if (child_a == 0) {
		log_file = argv[2];
		p = getpid();
		close(P1[1]);
		close(P2[1]);
		close(P3[0]);
		close(P4[0]);
		store_manager(P1[0], P3[1], P2[0], P4[1], argv[1], argv[2], p);

		/* Child A code */
	}
	child_b = fork();

	if (child_b == 0) {
		p = getpid();
		signal(SIGUSR1, my_handler);
		signal(SIGUSR2, my_handler);
		close(P1[0]);
		close(P3[1]);
		proc1(P3[0], P1[1], argv[3], p, argv[2]);
	}
	/* Child B code */
//	child_c = fork();
//
//	if (child_c == 0) {
//		close(P2[0]);
//		close(P4[1]);
		/*signal(SIGUSR1, my_handler);
		 signal(SIGUSR2, my_handler);
		 childC_id = getpid();*/
	}

	parent();
	//fclose(f_log);
	//fclose(f_ini);
	//fclose(f_proc1);
	//fclose(f_proc2);
	printf("Exit Program\n");
	kill(child_a, SIGKILL);
	kill(child_b, SIGKILL);
	kill(child_c, SIGKILL);
	return 0;
}

void store_manager(int read_p1, int write_p1, int read_p2, int write_p2,
		char* arg1, char* arg2, int pid) {
	TABLE *db;
	int i, scanRes, value;
	char line[SIZE], id[SIZE];
	int len, result;
	int n, up, x;
	int cnt = 0;
	char buff[MAXLINE];
	char inbuff[MAXLINE];
	time_t t;
	f_ini = fopen(arg1, "r");

	//Create Table
	while (fgets(line, sizeof(line), f_ini) != NULL) {
		line[strlen(line) - 1] = '\0';
		if (sscanf(line, "%s %d", id, &value)) {

		}
		db = malloc(sizeof(TABLE));
		if (db == NULL) {
			printf("Ran out of memory\n");
			return;
		}
		db->id = strdup(id);
		db->value = value;
		db->next = NULL;
		if (first != NULL) {
			last->next = db;
			last = db;
		} else {
			first = db;
			last = db;
		}
	}
	db = first;
	while (db != NULL) {
		printf("'%s' '%d'\n", db->id, db->value);
		db = db->next;
	}
	//End of Create Table
	//fclose(f_ini);
	printf("complete table\n");

	len = strlen(buff);
	if (buff[len - 1] == '\n')
		len--;

	//read from pipe
	while (done == 0) {
		//sleep(2);
		if ((n = read(read_p1, inbuff, MAXLINE)) > 0) {
			printf("in loop\n");
			inbuff[n] = '\0';
			if (inbuff) {
				printf("getting pipe in store %s\n", inbuff);
			} else {
				done++;
			}

			if (n > 0) {
				printf(" pipe %s", inbuff);
				len = strlen(inbuff);
				for (i = 0; inbuff[i] != '\0'; i++) {
					if (inbuff[i] == '\n') {
						inbuff[i] = '\0';
					}
				}

				message(arg2, "Store Manager", getpid(), inbuff, "received", 0,
						0, 0);
				char* token = strtok(inbuff, " ");
				//Read from db
				if (strcmp(token, "R") == 0) {
					printf("token %s\n", token);
					token = strtok(NULL, " ");
					for (i = 0; token[i] != '\0'; i++) {
						if (token[i] == '\n') {
							token[i] = '\0';
						}
					}
					result = table_read(token, db);
					printf("Value %d\n", result);
					if (result > 0) {
						exe1++;
						suc1++;
						char msg[SIZE];
						sprintf(msg, "%d R 0 %s %d", token, result);
						//strcat(msg, "R ");
						//strcat(msg, token);
						printf("Result: '%s' '%s'\n", inbuff, token);
						message(arg2, "Store Manager", getpid(), msg, "sent", 1,
								0, 0);
						x = strlen(msg);
						write(write_p1, msg, x);
					} else {
						exe1++;
						printf("Result: '%s' '%s'\n", inbuff, token);
						char msg[SIZE];
						sprintf(msg, "%d R 1 %s %d", getpid(), token, result);
						//strcat(msg, inbuff);
						//strcat(msg, " ");
						//strcat(msg, token);
						message(arg2, "Store Manager", getpid(), msg, "sent", 0,
								1, result);
						x = strlen(msg);
						write(write_p1, msg, x);
					}
				}
				printf("Total %d\n", exe1);
				//End read of db

				//Update db
				if (strcmp(token, "U") == 0) {
					token = strtok(NULL, " ");
					for (i = 0; token[i] != '\0'; i++) {
						if (token[i] == '\n') {
							token[i] = '\0';
						}
					}
					char* token2 = strtok(NULL, " ");
					for (i = 0; token2[i] != '\0'; i++) {
						if (token2[i] == '\n') {
							token2[i] = '\0';
						}
					}
					up = atoi(token2);
					result = table_update(token, db, up);
					if (result > 0) {
						exe2++;
						suc2++;
						message(arg2, "Store Manager", getpid(), inbuff, "sent",
								1, 0, result);
						char msg[SIZE] = "0 ";
						//strcat(msg, inbuff);
						x = strlen(msg);
						write(write_p1, msg, x);
					} else {
						exe1++;
						message(arg2, "Store Manager", getpid(), inbuff, "sent",
								0, 1, result);
						char msg[SIZE] = "1 ";
						//strcat(msg, inbuff);
						x = strlen(msg);
						write(write_p1, msg, x);
					}
				}
				//end of Update
			} else {
				printf("Try here\n");
				break;
			}
			sleep(3);
		}
		done++;
		break;
		/*else{
		 printf("yoyo\n");
		 break;
		 }
		 if(incoming > 0){
		 printf("Leaving store\n");
		 done++;
		 break;
		 }*/
		sleep(3);
	}
	printf("out of loop\n");
	exit(1);
	//end of reading from pipe
}

void proc1(int read_p, int write_p, char* file_open, int pid, char* log) {
	int n;
	int len, i;
	char buff[SIZE + 1];
	char inbuff[SIZE + 1];
	f_proc1 = fopen(file_open, "r");

	while (fgets(buff, SIZE, f_proc1) != NULL) {
		//sleep(1);
		printf("Sending '%s'\n", buff);
		n = strlen(buff);
		for (i = 0; buff[i] != '\0'; i++) {
			if (buff[i] == '\n') {
				buff[i] = '\0';
			}
		}

		if (n == 0) {
			break;
		}
		message(log, "Process 1", getpid(), buff, "sent", 0, 0, 0);
		write(write_p, buff, n);
		sleep(1);
		if ((n = read(read_p, inbuff, MAXLINE)) > 0) {
			if (n > 0) {
				message(log, "Proccess 1", pid, inbuff, "received", 0, 0, 0);
			}
		}
	}
	printf("Totttal %d", exe1);
	printf("close file\n");
	incoming = 1;
	fclose(f_proc1);
	while (1 == 1) {
		sleep(1);
	}

	return;

}

/*void proc2(int read_p, int write_p, char* file_open){
 int n;
 int len, i;
 char buff[SIZE+1];
 time_t t;
 f_proc2 = fopen(file_open, "r");
 //signal(SIGUSR2, my_handler);
 //childC_id = getpid();
 //printf("%d p2\n",getpid());

 while(fgets(buff, SIZE, f_proc2)){
 //sleep(1);
 n = strlen(buff);
 for(i=0;buff[i]!='\0';i++)
 {
 if(buff[i]=='\n')
 {
 buff[i]='\0';
 }
 }

 printf("P1 file: '%s'\n", buff);
 t = time(NULL);
 struct tm tm = *localtime(&t);
 fprintf(f_log,"Process 2 at: %d-%d-%d %d:%d:%d sent message: ",
 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
 tm.tm_hour, tm.tm_min, tm.tm_sec);
 //fprintf(f_log,"%s\n",buff);
 //sleep(1);
 write(write_p, buff, n);
 exe2++;
 //sleep(2);
 }
 //fclose(f_proc1);
 }*/

int table_update(char* id, TABLE* db, int up) {
	char* test;
	int value;
	db = first;
	while (db != NULL) {
		test = db->id;

		if (strcmp(test, id) == 0) {

			db->value = db->value + up;
			value = db->value;
			return value;
		}
		db = db->next;
	}
	return 0;
}

int table_read(char* id, TABLE* db) {
	char* test;
	int value;
	db = first;
	printf("Checking: %s\n", id);
	while (db != NULL) {
		test = db->id;

		if (strcmp(test, id) == 0) {
			printf("Returning %d ", db->value);
			value = db->value;
			return value;
		}
		db = db->next;
	}
	return 0;
}

void parent() {
	int entry;
	while (1) {

		printf("\n  1 or 2 to see stats:\n");
		scanf("%d", &entry);
		if (entry == 1) {
			kill(child_b, SIGUSR1);

		} else if (entry == 2) {
			kill(child_c, SIGUSR2);
		} else if (entry == 3) {
			return;
		} else {
			printf("Invalid data");
			entry = 3;
		}
		//sleep(1);
	}
}
void my_handler(int signum) {
	if (signum == SIGUSR1) {
		printf("My processID %d\n", getpid());
		printf("Reads: %d Successes   %d Total\n", suc1, exe1);
		printf("updates: %d Successes   %d Total\n", suc2, exe2);
	}
	//if(signum==SIGUSR2){
	//   printf("%d Successes   %d Totals\n",suc2,exe2);
	//}
}
void message(char* log, char* sender, int pid, char* buff, char* status,
		int success, int fail, int value) {
	f_log = fopen(log, "a+");
	printf(" loging: %s\n", buff);
	time_t t;
	char* result = "";

	if (success) {
		result = "1";
	}
	if (fail) {
		result = "0";
	}

	t = time(NULL);
	struct tm tm = *localtime(&t);
	fprintf(f_log, "%s at time: ", sender);
	fprintf(f_log, "%d-%d-%d %d:%d:%d ", tm.tm_year + 1900, tm.tm_mon + 1,
			tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(f_log, "%s %s ", status, buff);
	if (result > 0) {
		//fprintf(f_log," %d",value);
	}
	fprintf(f_log, "\n");
	fclose(f_log);
	return;
}
