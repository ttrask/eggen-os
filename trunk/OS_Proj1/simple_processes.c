/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * simple_processes.c
 *
 * Simple multi-process example.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>  

void do_one_thing(int *);
void do_another_thing(int *);
void do_wrap_up(int, int);

int   shared_mem_id;
int   *shared_mem_ptr;
int   *r1p;
int   *r2p;

int main(void)
{
//  pid_t  child1_pid, child2_pid;
  int  child1_pid, child2_pid;
  int  status;

  /* initialize shared memory segment */
  if ((shared_mem_id = shmget(IPC_PRIVATE, 2*sizeof(int), 0660)) == -1)
    printf("error in shmget"), exit(1);
  if ((shared_mem_ptr = (int *)shmat(shared_mem_id, (void *)0, 0)) == 
                                                 (void *)-1)
    printf("shmat failed"), exit(1);

  r1p = shared_mem_ptr;  
  r2p = (shared_mem_ptr + 1);

printf("the addrs, shared_mem_ptr=%u, r1p=%u, r2p=%u\n",shared_mem_ptr, r1p, r2p);

  *r1p = 0;
  *r2p = 0;

  if ((child1_pid = fork()) == 0) {
    /* first child */
    printf("child1_pid = %d\n",child1_pid);
    do_one_thing(r1p);
    return 0;
  } else if (child1_pid == -1) {
    printf("error in fork"), exit(1);
  }

  /* parent */
  if ((child2_pid = fork()) == 0) {
    /* second child */
    printf("child2_pid = %d\n",child2_pid);
    do_another_thing(r2p);
//    do_another_thing(r1p);
    exit (0);
  } else if (child2_pid == -1) {  
    printf("error in fork"), exit(1);
  }

  /* parent */
  
    printf("child1_pid = %d\n",child1_pid);
    printf("child2_pid = %d\n",child2_pid);
  if ((waitpid(child1_pid, &status, 0) == -1))
	printf("error in waitpid"), exit(1);
  if ((waitpid(child2_pid, &status, 0) == -1))
	printf("error in waitpid"), exit(1);
   
//  wait(NULL);

  do_wrap_up(*r1p, *r2p);

/* the following removes the allocated shared memory */
/* comment it out to see the shared memory           */
 
//    if (shmctl(shared_mem_id, IPC_RMID, 0) == -1) {
 //       printf("error in shmctl");
  //      exit(1);
 //  } 
  
 
  return 0; 
}

void do_one_thing(int *pnum_times)
{
  int i, j, x;
  
  for (i = 0;  i < 4; i++) {
    printf("doing one thing\n"); 
//    sleep(1);
    (*pnum_times)++;
  }

}

void do_another_thing(int *pnum_times)
{
  int i, j, x;
  
  for (i = 0;  i < 8; i++) {
   printf("doing another \n"); 
 //   sleep(1);
    (*pnum_times)++;
  }
// use the following to show that shared memory exists
// be sure to kill the process afterward
  printf("running\n"); 
// while(1);

}

void do_wrap_up(int one_times, int another_times)
{
  int total;

  total = one_times + another_times;
  printf("All done, one thing %d, another %d for a total of %d\n",
	one_times, another_times, total);
}

