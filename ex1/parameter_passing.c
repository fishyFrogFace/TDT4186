#include <stdio.h>

int a = 23;

void increment_with_value(int a, int b)
{
  a += b;
}

int main(void)
{
  int local_uninit;
  int local_init = 10;
  static int static_local_uninit;
  static int static_local_init = 5;
  //increment_with_value(a, 1);
  printf("%i", a);
  //return a;
  return 0;
}