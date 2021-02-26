#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

/* assuming we don't need to do error handling of
* IO operations exit(), printf() and sleep() 
* since their man pages do not contain error catching information
* of the type described in the assignment */

void timer(int seconds)
{
  sleep(seconds);
  printf("\a");
  printf("\nRING! child process %d rang after %d seconds\nEnter a delay in seconds: ", getpid(), seconds);
  exit(EXIT_SUCCESS);
}

int get_user_input(void)
{
  printf("\nEnter a delay in seconds: ");
  errno = 0;
  long seconds;

  /* "write programs to handle text streams,
  * because that is an universal interface" 
  * switched to handle strings here, to make error
  * handling of scanf a lot easier to deal with */
  char *user_input;
  scanf("%s", user_input);
  seconds = strtol(user_input, NULL, 10);

  if (errno != 0)
  {
    perror("strtol");
    exit(EXIT_FAILURE);
  }
  // we don't support timers with negative values
  // and you can't set a timer to 0 seconds
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
    errno = 0;
    int seconds = get_user_input();

    int terminated;
    while ((terminated = waitpid(-1, &wstatus, WNOHANG)) && terminated != 0 && terminated != -1)
    {
      printf("\nChild process %d terminated\n", terminated);
    }

    /* check if there was an error when calling waitpid
    * and that this error does not mean that there
    * simply was no child to wait for */
    if (terminated == -1 && errno != ECHILD)
    {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }

    int cpid = fork();
    if (cpid == 0)
    {
      timer(seconds);
    }
    // if the fork failed for some reason, exit the program
    else if (cpid == -1)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    else
    {
      printf("\nStarted child process with pid %d\n", cpid);
    }
  }

  return 0;
}