#include <stdio.h>

/* function to show bytes in memory, from location start to start+n*/
void show_mem_rep(char *start, int n)
{
  int i;
  for (i = 0; i < n; i++)
  {
    /* higher addresses will correspond with most significan digits in little endian */
    //printf("Address of start[i]: %p\n", &start[i]);
    printf(" %.2x\n", start[i]);
  }

  printf("\n");
}

/*Main function to call above function for 0x01234567*/
int main()
{
  int i = 0x01234567;
  printf("%X\n", i);
  show_mem_rep((char *)&i, sizeof(i));
  return 0;
}
