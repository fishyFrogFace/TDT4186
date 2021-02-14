#include <stdio.h>

int main()
{
  int seconds = 0;
  printf("How many seconds do you want to set the timer to? ");
  scanf("%d", &seconds);
  printf("You set the timer to ring in %i second(s)\n", seconds);
}