#include <stdio.h>

int a = 0, b = 1;

void bar(int b) {
  a = b;
}

void foo(int a, int b) {
  {
    int b = a;
    int a = a + b;
  }
  bar(a);
}

int main() {
  int b = a;
  {
    int a = 2;
    foo(a,b);
  }
  printf("%x", a);
  return 0;
}
