#include <stdio.h>
#include <stdlib.h>

int main_fork() {
	int j;
	int N = 5;
	int child1_pid[5];
	printf("creating N = %d child processes\n", N);
	for (j = 0; j < N; j++)
		if ((child1_pid[j] = fork()) == 0) {
			/* child executes in here */
			printf("child process runs here. j=%d, id=%d my real pid = %d\n\n",
					j, child1_pid[j], getpid());
			while (1 == 1)
				; // uncomment to verify process execution
			exit(0); // thanks to Avery, this was the whole problem
		} else {
			printf("parent process runs here. j=%d, id=%d parent pid %d \n\n",
					j, child1_pid[j], getpid());
		}
	return 0;
}
