#include <stdio.h>

int sum(int n)
{
  if (n == 1)
  {
    return n;
  }
  else
  {
    return n + sum(n - 1);
  }
}

int main()
{
  int n = 16106;
  printf("The sum of numbers from 1 to %i is %i", n, sum(n));
  return 0;
}