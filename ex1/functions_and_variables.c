#include <stdio.h>

const int c = 138384;
int d, counter;

unsigned int rec(unsigned int number)
{
  char array[1000];
  if (counter % 100000 == 0)
    printf("%i\n", counter);
  if (counter % 1000000000 == 0)
    printf("%c", array[999]);
  counter++;
  return rec(counter);
}

int main(void)
{
  int a = rec(c);
  printf("%d\n", a);
  return 0;
}