#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int has_initialized = 0;

// our memory area we can allocate from, here 64 kB
#define MEM_SIZE (64 * 1024)
uint8_t heap[MEM_SIZE];

// start and end of our own heap memory area
void *managed_memory_start;

// this block is stored at the start of each free and used block
struct mem_control_block
{
  int size;
  struct mem_control_block *next;
};

// pointer to start of our free list
struct mem_control_block *free_list_start;

void mymalloc_init()
{

  // our memory starts at the start of the heap array
  managed_memory_start = &heap;

  // allocate and initialize our memory control block
  // for the first (and at the moment only) free block
  struct mem_control_block *m = (struct mem_control_block *)managed_memory_start;
  m->size = MEM_SIZE - sizeof(struct mem_control_block);

  // no next free block
  m->next = (struct mem_control_block *)0;

  // initialize the start of the free list
  free_list_start = m;

  // We're initialized and ready to go
  has_initialized = 1;
}

void *mymalloc(long numbytes)
{
  if (has_initialized == 0)
  {
    mymalloc_init();
  }

  if (numbytes == 0)
  {
    return NULL;
  }
  /* add your code here! */
}

void myfree(void *firstbyte)
{

  /* add your code here! */
}

int main(int argc, char **argv)
{
  /* Allocation tests */
  void *result = mymalloc(0);

  printf("Returns NULL if asked to allocate 0 bytes: ");
  if (result == NULL)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("When nothing is allocated, the first mem_control_block has length 64*1024 minus the first mem_control_block: ");
  if (free_list_start->size == MEM_SIZE - sizeof(struct mem_control_block))
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("When one block is allocated, our free block spans the rest of the memory: ");

  mymalloc(40);

  if (free_list_start->size == MEM_SIZE - sizeof(struct mem_control_block))
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* allocation
  * 2. can allocate two blocks (with no deallocation in between)
  * 3. allocate something in first large enough space |000000|1111|000..
  * 4. don't allocate something in first free space when first free space is not large enough |00|1111|000...
  * 5. if asked for more space than we have in any free space, return something clever
  * 
  * deallocation
  * 1. 1 thing in beginning that should be deallocated
  * 2. two things in start where second should be deallocated
  * 3. deallocate block with free block in front
  * 4. deallocate block with allocated block in front and other free blocks in front
  * 5. deallocate a block that will then be the last free block
  * 6. opposite of 5
  * */
}
