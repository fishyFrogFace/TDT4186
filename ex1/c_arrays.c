#include <stdio.h>
#include <string.h>

int main(void)
{
  int foo = 0;
  char s[12];
  char *t = "01234567890123";
  printf("foo %p\n  s %p\n", &foo, s);
  // to allow stack smashing: gcc -Wall -fno-stack-protector -o c_arrays.o ex1/c_arrays.c
  strcpy(s, t);
  printf("foo = %d\n", foo);
}