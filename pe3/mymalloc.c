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

  /* allocate and initialize our memory control block
  * for the first (and at the moment only) free block
  * edit: size includes the 16 bytes of the struct */
  struct mem_control_block *m = (struct mem_control_block *)managed_memory_start;
  m->size = MEM_SIZE;

  /* no next free block
  * edit: we felt that NULL was more readable
  * than (struct mem_control_block *)0 */
  m->next = NULL;

  // initialize the start of the free list
  free_list_start = m;

  // We're initialized and ready to go
  has_initialized = 1;
}

void *find_suitable_block(unsigned short numbytes, struct mem_control_block *current)
{
  if (current == NULL)
  {
    return NULL;
  }
  else if (current->size >= numbytes + sizeof(short))
  {
    return current;
  }
  else if (current->next == NULL)
  {
    return NULL;
  }
  else
  {
    return find_suitable_block(numbytes, current);
  }
}

void *mymalloc(long numbytes)
{
  if (has_initialized == 0)
  {
    mymalloc_init();
  }

  // determine how much space we need and if we have this space available
  long size_of_new_block = numbytes + sizeof(short);
  void *first_suitable = find_suitable_block(numbytes, free_list_start);

  /* if no space is requested or we don't have this amount of space
  * then return NULL */
  if (numbytes == 0 || first_suitable == NULL)
  {
    return NULL;
  }
  //if the first suitable free block is the first free block
  else if (first_suitable == free_list_start)
  {
    // if the suitable block is larger than the block we want
    if (free_list_start->size > size_of_new_block)
    {
      // there is room for a struct in the remaining free block
      if (free_list_start->size - size_of_new_block > 16)
      {
        struct mem_control_block *m = (void *)free_list_start + size_of_new_block;
        m->size = free_list_start->size - size_of_new_block;

        unsigned short *block_size = (unsigned short *)free_list_start;
        *block_size = size_of_new_block;

        free_list_start = m;

        return block_size + 1;
      }
      /* If there is no room for a struct in the remaining free block
      * we add the extra space to the block we are allocating.
      * This way we can still keep track of it without creating
      * a new structure to keep track of small deallocated pieces. */
      else
      {
        unsigned short *block_size = (unsigned short *)free_list_start;
        unsigned short size_of_free_block = free_list_start->size;
        free_list_start = free_list_start->next;
        *block_size = size_of_free_block;

        return block_size + 1;
      }
    }
    // if the suitable block is equal to the block we want
    else
    {
      unsigned short *block_size = (unsigned short *)free_list_start;
      free_list_start = free_list_start->next;
      *block_size = size_of_new_block;

      return block_size + 1;
    }
  }
  // all other possibilities
  else
  {
    printf("NOT IMPLEMENTED");
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
  /* Simple allocation tests */
  printf("SIMPLE ALLOCATION:\n");

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
  if (free_list_start->size == MEM_SIZE)
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

  long numbytes = 40;
  result = mymalloc(numbytes);

  if (free_list_start->size == MEM_SIZE - numbytes - sizeof(short))
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("When one block is allocated, mymalloc returns managed_memory_start + sizeof(short): ");

  if (result == managed_memory_start + sizeof(short))
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    printf("managed_memory_start + sizeof(short): %p\n", managed_memory_start + sizeof(short));
    printf("result: %p\n", result);
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("When one block is allocated, the free block struct is located after the allocated block: ");

  if (free_list_start == managed_memory_start + sizeof(short) + numbytes)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    printf("managed_memory_start + sizeof(short) + numbytes: %p\n", managed_memory_start + sizeof(short) + numbytes);
    printf("free_list_start: %p\n", free_list_start);
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Sets the first 2 bytes of the allocated block to the size of the block: ");

  unsigned short *block_size = managed_memory_start;
  if (*block_size == numbytes + 2)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  /* A short is big enough to store the size of our memory,
  * so it makes sense to use a short to store the length of a block
  * to save space.
  * That means we have to check if the long we send in is too big to
  * fit in a short or too big to allocate, even if we used all available
  * memory */
  printf("Returns NULL if it receives a long that is larger than max short - 2 bytes: ");

  long numbytes2 = 64 * 1024 - 1;
  result = mymalloc(numbytes2);

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

  printf("Can allocate a block that takes all remaining memory: ");

  long numbytes3 = MEM_SIZE - numbytes - 2 * sizeof(short);
  result = mymalloc(numbytes3);

  if (result == managed_memory_start + 2 * sizeof(short) + numbytes && free_list_start == NULL)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can allocate a block that creates a free block smaller than 16 bytes: ");

  mymalloc_init();
  mymalloc(numbytes);
  long numbytes4 = MEM_SIZE - numbytes - 2 * sizeof(short) - 15;
  result = mymalloc(numbytes4);

  if (result == managed_memory_start + 2 * sizeof(short) + numbytes && free_list_start == NULL)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can allocate a second block, without overwriting the first block: ");

  mymalloc_init();
  mymalloc(numbytes);
  long numbytes5 = 64;
  result = mymalloc(numbytes5);

  if (result == managed_memory_start + sizeof(short) * 2 + numbytes)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */
  /* Tests for find_suitable_block */
  printf("\nFIND SUITABLE BLOCK:\n");

  printf("Returns first suitable block when that block is passed to the function: ");

  result = find_suitable_block(40, free_list_start);
  if (result == free_list_start)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  printf("Returns NULL if there are no suitable blocks: ");

  mymalloc_init(); // reset state
  long numbytes6 = 64 * 1024 - 50;
  mymalloc(numbytes6);
  result = find_suitable_block(100, free_list_start);

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

  /* allocation
  * 4. don't allocate something in first free space when first free space is 
  * not large enough |00|1111|000... (with multiple free spaces)
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
