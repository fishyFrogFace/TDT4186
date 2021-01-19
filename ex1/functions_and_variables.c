#include <stdio.h>

const int c = 138384;
int d, counter;

unsigned int rec(unsigned int number)
{
  if (counter % 10000 == 0)
    printf("%i\n", counter);
  counter++;
  return rec(counter);
}

int main(void)
{
  int a = rec(c);
  printf("%d\n", a);
  return 0;
}