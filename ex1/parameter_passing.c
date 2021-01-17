#include <stdio.h>

int a = 23;

void increment_with_value(int a, int b)
{
  a += b;
}

int main(void)
{
  //increment_with_value(a, 1);
  printf("%i", a);
  //return a;
  return 0;
}