#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

/*
** THIS CODE IS HANDLING THE PASSING VARIABLE $? THROUGH THE PIPES
** The evolution of the code for a better understanding is given
** code v0 handle a fixed number of pipes the simplest way
** code v1 some N function are added
** code v2 full N pipes handled
*/

//V1 & V2 - functions
void  init_pipes(int nb_pipes, int *pipes)
{
  int i;

  i = 0;
  while (i < nb_pipes)
  {
    pipe(pipes + i);
    i += 2;
  }
}

void  close_pipes(int nb_pipes, int *pipes)
{
  while (nb_pipes--)
    close(pipes[nb_pipes]);
}

void  wait_pipes(int nb_cmd, pid_t *pid, int *ret)
{
  int i;

  i = 0;
  while (i < nb_cmd)
  {
    waitpid(pid[i], ret, 0);
    i++;
  }
}

//V2 - functions
void do_dup(int j, int nb_cmd, int *pipes)
{
  if (j > 0)
    dup2(pipes[j * 2 - 2], 0);
  if (j < nb_cmd - 1)
    dup2(pipes[j * 2 + 1], 1);
}


//V2 - N CMDS
void do_pipe(char ***all, int nb_cmd, int *ret)
{
  pid_t *pid;
  int   pipes[nb_cmd * 2 - 2];
  int   j = -1;

  if (!(pid = (pid_t *)malloc(sizeof(int) * nb_cmd + 1)))
    return ;
  init_pipes(nb_cmd * 2 - 2, pipes);
  while (j++ < nb_cmd)
  {
     if (!(pid[j] = fork()))
     {
       do_dup( j, nb_cmd, pipes);
       close_pipes(nb_cmd * 2 - 2, pipes);
       if(execvp(*all[j], all[j]))
	exit(-1);
     }
  }
  close_pipes(nb_cmd * 2 - 2, pipes);
  wait_pipes(nb_cmd, pid, ret);
  free(pid);
}








int main(int argc, char **argv)
{
  int ret;

  // cat pipe.c | grep pipe | cut -b 1-10 | head -n 5
  char *cat_args[] = {"cat", "pipe.c", NULL};
  char *grep_args[] = {"grep", "pipe", NULL};
  char *cut_args[] = {"cut", "-b", "1-10", NULL};
  char *head_args[] = {"head", "-n", "5", NULL};
  char **all[4] = {cat_args, grep_args, cut_args, head_args};
  do_pipe(all, 4, &ret);
  printf("1 : %d\tEXPECTED : 0\n\n", ret);

  // cat bonjour | grep grep | cut -b 1-10 | head -n 5
  char *test1[] = {"cat", "bonjour", NULL};
  char *test2[] = {"grep", "grep", NULL};
  char *test3[] = {"cut", "-b", "1-10", NULL};
  char *test4[] = {"head", "-n", "5", NULL};
  char **all1[4] = {test1, test2, test3, test4};
  do_pipe(all1, 4, &ret);
  printf("2 : %d\tEXPECTED : 0\n\n", ret);

  // cat pipe.c | grep grep | cut -b 1-10 | test
  char *test5[] = {"cat", "pipe.c", NULL};
  char *test6[] = {"grep", "grep", NULL};
  char *test7[] = {"cut", "-b", "1-10", NULL};
  char *test8[] = {"test", NULL};
  char **all2[4] = {test5, test6, test7, test8};
  do_pipe(all2, 4, &ret);
  printf("3 : %d\tEXPECTED : 1\n\n", ret);

  // cat pipe.c | grep grep | cut -b 1-10 | head -n 5 ; 
  char *test9[] = {"cat", "pipe.c", NULL};
  char *test10[] = {"grep", "grep", NULL};
  char *test11[] = {"cut", "-b", "1-10", NULL};
  char *test12[] = {"head", "-n", "5", NULL};
  char **all3[4] = {test9, test10, test11, test12};
  do_pipe(all3, 4, &ret);
  printf("4 : %d\tEXPECTED : 0\n\n", ret);

  // cat pipe.c | grep grep | test -b 1-10 | head -n 5
  char *test13[] = {"cat", "pipe.c", NULL};
  char *test14[] = {"grep", "grep", NULL};
  char *test15[] = {"test", "-b", "1-10", NULL};
  char *test16[] = {"head", "-n", "5", NULL};
  char **all4[4] = {test13, test14, test15, test16};
  do_pipe(all4, 4, &ret);
  printf("5 : %d\tEXPECTED : 0\n\n", ret);

  // cat pipe.c | grep grep | cut -b 1-10 | cd bonjour
  char *test17[] = {"cat", "pipe.c", NULL};
  char *test18[] = {"grep", "grep", NULL};
  char *test19[] = {"cut", "-b", "1-10", NULL};
  char *test20[] = {"cd", "bonjour", NULL};
  char **all5[4] = {test17, test18, test19, test20};
  do_pipe(all5, 4, &ret);
  printf("6 : %d\tEXPECTED : 1\n\n", ret);
  
  char *test21[] = {"cat", NULL};
  char *test22[] = {"cat", NULL};
  char *test23[] = {"ls", NULL};
  char **all6[3] = {test21, test22, test23};
  do_pipe(all6, 3, &ret);
  printf("7 : %d\tEXPECTED : 1\n\n", ret);
}
