#include <stdio.h>
#include <unistd.h>

void set_alarm(int seconds)
{
  sleep(seconds);
  printf("\a");
  printf("Alarm going off!\n");
}

int main()
{
  int seconds = 0;
  printf("How many seconds do you want to set the timer to? ");
  scanf("%d", &seconds);
  printf("You set the timer to ring in %i second(s)\n", seconds);
  set_alarm(seconds);
  return 0;
}