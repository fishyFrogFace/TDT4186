#include <stdio.h>
#include <string.h>

int main(void)
{
  int foo = 0;
  char s[12];
  char *t = "01234567890123";
  printf("foo %p\n  s %p\n", &foo, s);
  strcpy(s, t);
  printf("foo = %d\n", foo);
}