#include <stdio.h>
#include <stdlib.h>

int main()
{
  int size = 2;
  // Dynamically allocate memory using malloc()
  int *arr1 = (int *)malloc(size * sizeof(int));
  int *arr2 = (int *)malloc(size * sizeof(int));
  int arr3[2] = {1, 2};
  int arr4[2] = {1, 2};

  if (arr1 == NULL)
  {
    printf("Memory not allocated.\n");
    exit(0);
  }
  else
  {
    printf("Memory successfully allocated using malloc.\n");

    printf("the address of the first array (heap): %p\n", arr1);   //94091227284128
    printf("the address of the second array (heap): %p\n", arr2);  //94091227284160
    printf("the address of the third array (stack): %p\n", arr3);  //140736144080856
    printf("the address of the fourth array (stack): %p\n", arr4); //140736144080864

    //insert values into array
    for (int i = 0; i < size; ++i)
    {
      arr1[i] = i + 1;
    }

    printf("The elements of the array are: ");
    for (int i = 0; i < size; ++i)
    {
      printf("%d, ", arr1[i]);
    }
    printf("\n");
  }

  return 0;
}