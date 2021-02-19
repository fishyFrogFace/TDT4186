#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

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
  errno = 0;
  long seconds;
  char *user_input;
  int num_parsed_inputs = scanf("%s", user_input);
  seconds = strtol(user_input, NULL, 10);

  printf("Now printing seconds: %d\n", seconds);

  if (errno != 0)
  {
    perror("strtol");
    exit(EXIT_FAILURE);
  }
  else if (seconds <= 0)
  {
    printf("Invalid input\n");
    exit(EXIT_FAILURE);
  }

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