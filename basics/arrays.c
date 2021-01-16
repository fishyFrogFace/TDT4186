#include <stdio.h>
// how/what is the heap? not a heap! a pool of memory, a messy place
// how much memory do we have available for our program? nobody knows
// Camilla er fin

int main()
{
  int arr[4] = {1, 98, 3, 7};
  int *ptr = (int *)((unsigned long long)&arr + (3 * 4));
  printf("%i\n", arr[3]);
  printf("%i\n", *ptr);
  return 0;
}