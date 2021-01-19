#include <stdio.h>
#include <string.h>

void original(void)
{
  int foo = 0;
  char s[12];
  char *t = "01234567890123";
  printf("foo %p\n  s %p\n", &foo, s);
  strcpy(s, t);
  printf("foo = %d\n", foo);
}

void modified(void)
{
  static int foo = 0;
  char s[12];
  char *t = "01234567890123";
  printf("foo %p\n  s %p\n", &foo, s);
  strcpy(s, t);
  printf("foo = %d\n", foo);
}

// to allow stack smashing: gcc -Wall -fno-stack-protector -o c_arrays.o ex1/c_arrays.c
int main(void)
{
  printf("Original code:\n");
  original();
  printf("\nModified with static variable:\n");
  modified();
  return 0;
}