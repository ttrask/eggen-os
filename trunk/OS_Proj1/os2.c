/*
 * os2.c
 *
 *  Created on: Sep 26, 2012
 *      Author: tom
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

int main(void) {
	void sigint_handler(int sig); /* prototype */
	char s[200];
	/* set up the handler */
	if (signal(SIGINT, sigint_handler) == SIG_ERR) {
		perror("signal");
		exit(1);
	}
	printf("Enter a string:\n");
	if (gets(s) == NULL)
		perror("gets");
	else
		printf("You entered: \"%s\"\n", s);
	return 0;
}

/* this is the handler */
void sigint_handler(int sig) {
	printf("Not this time!\n");
}

