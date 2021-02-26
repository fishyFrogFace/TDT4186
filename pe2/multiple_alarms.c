#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void timer(int seconds)
{
  sleep(seconds);
  /* in case an alarm rings, we want to show the prompt again
  * in case the use does not understand that a new alarm can still be set */
  printf("\nRING! child process %d rang after %d seconds\nEnter a delay in seconds: ", getpid(), seconds);
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
    int cpid = fork();
    // if we are in child, set a timer
    if (cpid == 0)
    {
      timer(seconds);
    }
    // if we are in parent, print pid of child
    else
    {
      printf("\nStarted child process with pid %d\n", cpid);
    }
  }

  return 0;
}