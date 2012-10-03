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
#include <sys/wait.h>

int i = 0;
int producerCount = 0;
int consumerCount = 0;
int semCount = 0;
int queueSize = 10;
int itemCount = 0;

void SetCLIValues(char*, int);
int isNumeric(char*);

int main(int argc, char *argv[]) {


	//assume first argument is the application name
	if (argc < 4) {
		printf("Error: Invalid input");
		exit(0);
	}

	for(i=1;i<argc;i++){
		//printf("CLI Value:%s\n", argv[i]);

		SetCLIValues(argv[i], i );
	}
	//Gets CLI Values.  Tanks if values not provided.


	return 0;
}

void SetCLIValues(char *arg, int i) {

		if (isNumeric(arg)!=0)
		{
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
				break;
			case 2:
				printf("Error: Invalid consumer value");
				break;
			case 3:
				printf("Error: Invalid item count value");
				break;
			}
			exit(0);
		}


	return;
}

int isNumeric (char * s)
{
    if (s == NULL || *s == '\0' || isspace(*s))
      return 0;
    char * p;
    strtod (s, &p);
    return *p == '\0';
}
