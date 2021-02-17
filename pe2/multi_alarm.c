#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void timer(int seconds)
{
  sleep(seconds);
  printf("\a");
  printf("\nRING! child process %d rang after %d seconds\n", getpid(), seconds);
  exit(0);
}

int get_user_input(void)
{
  printf("\nEnter a delay in seconds: ");
  int seconds;
  scanf("%d", &seconds);
  return seconds;
}

int main(void)
{
  int wstatus;
  while (1)
  {
    int seconds = get_user_input();
    int terminated = waitpid(-1, &wstatus, WNOHANG);
    if (terminated != 0 && terminated != -1)
      printf("\nChild process %d terminated", terminated);
    int cpid = fork();
    if (cpid == 0)
    {
      timer(seconds);
    }
    else
    {
      printf("\nStarted child process with pid %d\n", cpid);
    }
  }

  return 0;
}