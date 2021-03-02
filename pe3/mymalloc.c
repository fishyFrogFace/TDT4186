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

struct free_blocks
{
  struct mem_control_block *prev;
  struct mem_control_block *next;
};

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

struct free_blocks find_suitable_block(unsigned short numbytes, struct mem_control_block *prev, struct mem_control_block *current)
{
  if (current == NULL)
  {
    struct free_blocks blocks = {NULL, NULL};
    return blocks;
  }
  else if (current->size >= numbytes + sizeof(short))
  {
    struct free_blocks blocks = {prev, current};
    return blocks;
  }
  else if (current->next == NULL)
  {
    struct free_blocks blocks = {NULL, NULL};
    return blocks;
  }
  else
  {
    return find_suitable_block(numbytes, current, current->next);
  }
}

void *mymalloc(long numbytes)
{
  if (has_initialized == 0)
  {
    mymalloc_init();
  }

  long size_of_new_block = numbytes + sizeof(short);

  struct free_blocks blocks = find_suitable_block(numbytes, NULL, free_list_start);
  struct mem_control_block *first_suitable = blocks.next;
  unsigned short *block_size = (unsigned short *)first_suitable;
  struct mem_control_block **pointer_to_prev_or_free_list_start =
      (void *)first_suitable == free_list_start ? &free_list_start : &blocks.prev;

  /* If no space is requested or we don't have this amount of space
  * then return NULL.
  * Also return NULL if we try to allocate less than 16 bytes,
  * because we want to have space for a mem_control_block if
  * we deallocate the block again. */
  if (numbytes == 0 || first_suitable == NULL || numbytes + sizeof(short) < 16)
  {
    return NULL;
  }
  // if the suitable block is larger than the block we want
  if (free_list_start->size > size_of_new_block)
  {
    // there is room for a struct in the remaining free block
    if (first_suitable->size - size_of_new_block > 16)
    {
      struct mem_control_block *m = (void *)first_suitable + size_of_new_block;
      m->size = first_suitable->size - size_of_new_block;
      m->next = first_suitable->next;

      *block_size = size_of_new_block;

      *pointer_to_prev_or_free_list_start = m;
    }
    /* If there is no room for a struct in the remaining free block
      * we add the extra space to the block we are allocating.
      * This way we can still keep track of it without creating
      * a new structure to keep track of small deallocated pieces. */
    else
    {
      unsigned short size_of_free_block = first_suitable->size;
      *pointer_to_prev_or_free_list_start = first_suitable->next;
      *block_size = size_of_free_block;
    }
  }
  // if the suitable block is equal to the block we want
  else
  {
    *pointer_to_prev_or_free_list_start = first_suitable->next;
    *block_size = size_of_new_block;
  }
  return block_size + 1;
}

struct free_blocks find_free_blocks(void *firstbyte, struct mem_control_block *current)
{
  struct free_blocks blocks = {NULL, NULL};
  // if there are no free blocks, return NULL
  if (current == NULL)
  {
    return blocks;
  }
  // if current is the previous free block, return that
  else if ((void *)current < firstbyte && (current->next == NULL || firstbyte < (void *)current->next))
  {
    blocks.prev = current;
    blocks.next = current->next;
    return blocks;
  }
  // if there is no previous free block
  else if (firstbyte < (void *)current)
  {
    blocks.prev = NULL;
    blocks.next = current;
    return blocks;
  }
  // if there are many preceding free blocks, keep iterating
  else
  {
    return find_free_blocks(firstbyte, current->next);
  }
}

void myfree(void *firstbyte)
{
  unsigned short *size_of_block = (unsigned short *)(firstbyte - 2);
  //printf("\nsize of block: %d\n", *size_of_block);

  struct free_blocks blocks = find_free_blocks(firstbyte, free_list_start);
  //printf("(%p, %p)\n", blocks.prev, blocks.next);

  // pointer needed if we do not have to merge with preceding free block
  struct mem_control_block *m = (struct mem_control_block *)size_of_block;

  // there are no free blocks
  if (blocks.prev == NULL && blocks.next == NULL)
  {
    m->size = *size_of_block;
    m->next = NULL;

    free_list_start = m;
  }
  // there is no free block before the block we want to free
  else if (blocks.prev == NULL)
  {
    // check if there's a free block immediately after
    if (blocks.next == (void *)size_of_block + *size_of_block)
    {
      // merge the two blocks
      m->size = *size_of_block + blocks.next->size;
      m->next = blocks.next->next;
    }
    else
    {
      m->size = *size_of_block;
      m->next = blocks.next;
    }

    free_list_start = m;
  }
  // there is no free block after the block we want to free
  else if (blocks.next == NULL)
  {
    void *last_address_of_prev = (void *)blocks.prev + blocks.prev->size;
    // check if there's a free block immediately before
    if (last_address_of_prev == size_of_block)
    {
      // add size of freed block to preceding free block
      blocks.prev->size = blocks.prev->size + *size_of_block;
    }
    else
    {
      m->size = *size_of_block;
      m->next = NULL;

      blocks.prev->next = m;
    }
  }
  // there are at least one free block before and after the block we want to free
  else
  {
    void *last_address_of_prev = (void *)blocks.prev + blocks.prev->size;
    // check if there's a free block immediately on both sides
    if (blocks.next == (void *)size_of_block + *size_of_block && last_address_of_prev == size_of_block)
    {
      // add freed block and succeeding free block to preceding free block
      blocks.prev->size = blocks.prev->size + *size_of_block + blocks.next->size;
      blocks.prev->next = blocks.next->next;
    }
    // check if there's a free block immediately after
    else if (blocks.next == (void *)size_of_block + *size_of_block)
    {
      m->size = *size_of_block + blocks.next->size;
      m->next = blocks.next->next;

      blocks.prev->next = m;
    }
    // check if there's a free block immediately before
    else if (last_address_of_prev == size_of_block)
    {
      blocks.prev->size = blocks.prev->size + *size_of_block;
    }
    // no block on either side that needs merging
    else
    {
      m->size = *size_of_block;
      m->next = blocks.next;

      blocks.prev->next = m;
    }
  }
}

void print_free_list(struct mem_control_block *current)
{
  if (current == NULL)
  {
    printf("\nEnd of free list\n");
  }
  else
  {
    printf("\nFree block address: %p, size: %d, next: %p", current, current->size, current->next);
    print_free_list(current->next);
  }
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

  mymalloc_init(); // reset state
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

  struct free_blocks first_suitable_result = find_suitable_block(40, NULL, free_list_start);
  if ((void *)first_suitable_result.next == free_list_start && (void *)first_suitable_result.prev == NULL)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  printf("Returns NULL if there are no suitable blocks: ");

  mymalloc_init();
  long numbytes6 = 64 * 1024 - 50;
  mymalloc(numbytes6);
  first_suitable_result = find_suitable_block(100, NULL, free_list_start);

  if ((void *)first_suitable_result.next == NULL && (void *)first_suitable_result.prev == NULL)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  printf("Recursively finds suitable block and it's predecessor: ");

  mymalloc_init();
  mymalloc(20);
  mymalloc(30);
  result = mymalloc(40);
  mymalloc(30);
  void *result2 = mymalloc(30);
  void *result3 = mymalloc(50);
  myfree(result);
  myfree(result2);

  first_suitable_result = find_suitable_block(41, NULL, free_list_start);

  if ((void *)first_suitable_result.next == result3 + 50 &&
      (void *)first_suitable_result.prev == result2 - 2)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */
  /* Simple deallocation tests */

  printf("\nDEALLOCATION:\n");

  printf("Can deallocate a block when memory is full: ");

  mymalloc_init();
  mymalloc(numbytes);
  result = mymalloc(numbytes3);

  myfree(result);

  if (result - sizeof(short) == free_list_start)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can merge a freed block with a succeeding free block: ");

  mymalloc_init();
  mymalloc(40);
  result = mymalloc(20);
  myfree(result);

  if (result - sizeof(short) == free_list_start)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can merge a freed block with a preceding free block: ");

  mymalloc_init();
  mymalloc(numbytes);
  void *first_to_free = mymalloc(20);
  void *second_to_free = mymalloc(30);
  mymalloc(MEM_SIZE - numbytes - 50 - 4 * sizeof(short));
  myfree(first_to_free);
  myfree(second_to_free);

  if (first_to_free - sizeof(short) == free_list_start)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can merge a freed block with blocks on both sides: ");

  mymalloc_init();
  mymalloc(40);
  first_to_free = mymalloc(20);
  void *third_to_free = mymalloc(30);
  second_to_free = mymalloc(20);
  mymalloc(MEM_SIZE - 110 - 4 * sizeof(short));
  myfree(first_to_free);
  myfree(second_to_free);
  myfree(third_to_free);

  if (first_to_free - sizeof(short) == free_list_start && free_list_start->next == NULL)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */
  /* More extensive tests that combines allocation and deallocation */

  printf("\nCOMPOUND TESTS:\n");

  printf("Can skip a free space and allocate in next when the first free space is not large enough: ");

  mymalloc_init();
  mymalloc(40);
  void *block_to_free = mymalloc(20);
  mymalloc(30);

  myfree(block_to_free);

  result = mymalloc(21);

  if (result == managed_memory_start + 90 + 4 * sizeof(short) && (void *)free_list_start < result)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can allocate something in first free space when first free space is large enough: ");

  mymalloc_init();
  mymalloc(40);
  block_to_free = mymalloc(20);
  mymalloc(30);

  myfree(block_to_free);

  result = mymalloc(20);

  if (result == managed_memory_start + 40 + 2 * sizeof(short) && result < (void *)free_list_start)
  {
    printf("YES\n");
  }
  else
  {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */
}
