#include <stdio.h>

void intPointer()
{
  int a = 5000000;
  int *aPtr = &a;
  int b = *aPtr;
  printf("int that is 5: %i\n", b);
  printf("pointer to that int: %p\n", aPtr);
  printf("address of that int: %p\n", &a);
  printf("sizeof that int: %lu\n\n", sizeof(a));
}

void charPointer()
{
  char a = 'p';
  char b = 'c';
  char *aPtr = &a;
  char *bPtr = &b;
  printf("char that is 'p': %i\n", a);
  printf("pointer to that char: %p\n", aPtr);
  printf("char that is 'c': %i\n", b);
  printf("pointer to 'c': %p\n", bPtr);
  printf("sizeof 'c': %lu\n", sizeof(a));
  printf("the address of the pointer to 'p': %p\n", &aPtr);
  printf("the address of the pointer to 'c': %p\n", &bPtr);
  /*  each pointer has a sizeof 8 because:
    Prelude> length "0000000F8736F7E7"
    16
    Prelude> 16^16 --the number of addresses from length of address
    18446744073709551616
    Prelude> 2^64 --the number of addresses on a 64-bit machine
    18446744073709551616 <- it's the same, yay! */
  printf("sizeof pointer to 'c': %lu\n\n", sizeof(aPtr));
}

int main()
{
  intPointer();
  charPointer();
  return 0;
}