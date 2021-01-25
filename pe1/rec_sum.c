#include <stdio.h>

char char_global = 'c';
char char_unitialized_global;
static char char_static_global = 'c';
static char char_static_unitialized_global;

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

void print_locals()
{
  char char_local = 'c';
  char char_unitialized_local;
  static char char_static_local = 'c';
  static char char_static_unitialized_local;

  printf("Local variables: \n");
  printf("Address of char_local is %p\n", &char_local);
  printf("Address of char_unitialized_local is %p\n", &char_unitialized_local);
  printf("Address of char_static_local is %p\n", &char_static_local);
  printf("Address of char_static_unitialized_local is %p\n\n", &char_static_unitialized_local);
}

void print_globals()
{
  printf("Global variables: \n");
  printf("Address of char_global is %p\n", &char_global);
  printf("Address of char_unitialized_global is %p\n", &char_unitialized_global);
  printf("Address of char_static_global is %p\n", &char_static_global);
  printf("Address of char_static_unitialized_global is %p\n\n", &char_static_unitialized_global);
}

int main()
{
  print_globals();
  print_locals();

  // these two statements segfaults in combination, but not separately
  // int int_lst[500000];
  // printf("The sum of numbers from 1 to 250000 is %i\n", sum(250000));

  printf("The sum of numbers from 1 to 196607 is %i\n", sum(196607)); //2147385344
  printf("The sum of numbers from 1 to 196608 is %i\n", sum(196608)); //-2147385344
  // printf("The sum of numbers from 1 to 2500000 is %i\n", sum(2500000)); //segfaults
  return 0;
}
