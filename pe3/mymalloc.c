#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int has_initialized = 0;

// our memory area we can allocate from, here 64 kB
#define MEM_SIZE (64 * 1024)
uint8_t heap[MEM_SIZE];

// start and end of our own heap memory area
void *managed_memory_start;

// this block is stored at the start of each free and used block
struct mem_control_block {
  int size;
  struct mem_control_block *next;
};

// pointer to start of our free list
struct mem_control_block *free_list_start;

struct free_blocks {
  struct mem_control_block *prev;
  struct mem_control_block *next;
};

void mymalloc_init() {

  // our memory starts at the start of the heap array
  managed_memory_start = &heap;

  /* allocate and initialize our memory control block
   * for the first (and at the moment only) free block
   * edit: size includes the 16 bytes of the struct */
  struct mem_control_block *m =
      (struct mem_control_block *)managed_memory_start;
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

struct free_blocks find_suitable_block(long numbytes,
                                       struct mem_control_block *prev,
                                       struct mem_control_block *current) {
  if (current == NULL) {
    struct free_blocks blocks = {NULL, NULL};
    return blocks;
  } else if (current->size >= numbytes) {
    struct free_blocks blocks = {prev, current};
    return blocks;
  } else if (current->next == NULL) {
    struct free_blocks blocks = {NULL, NULL};
    return blocks;
  } else {
    return find_suitable_block(numbytes, current, current->next);
  }
}

int calc_rounded(long numbytes) {
  long rounded = (numbytes + sizeof(long) + 8 - 1) & -8;
  return rounded < 16 ? 16 : rounded;
}

void *mymalloc(long numbytes) {
  if (has_initialized == 0) {
    mymalloc_init();
  }

  /* add the size of a long and round up to a multiple of 8
  * but minimum 16 bytes, so we will always have room for a
  * mem_control_block when freeing */
  long size_of_new_block = calc_rounded(numbytes);

  struct free_blocks blocks =
      find_suitable_block(size_of_new_block, NULL, free_list_start);
  struct mem_control_block *first_suitable = blocks.next;
  long *block_size = (long *)first_suitable;
  struct mem_control_block **pointer_to_prev_or_free_list_start =
      (void *)first_suitable == free_list_start ? &free_list_start
                                                : &blocks.prev;

  /* If no space is requested or we don't have this amount of space
   * then return NULL. */
  if (numbytes <= 0 || first_suitable == NULL) {
    return NULL;
  }
  // if the suitable block is larger than the block we want
  if (free_list_start->size > size_of_new_block) {
    // there is room for a struct in the remaining free block
    if (first_suitable->size - size_of_new_block > 16) {
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
    else {
      long size_of_free_block = first_suitable->size;
      *pointer_to_prev_or_free_list_start = first_suitable->next;
      *block_size = size_of_free_block;
    }
  }
  // if the suitable block is equal to the block we want
  else {
    *pointer_to_prev_or_free_list_start = first_suitable->next;
    *block_size = size_of_new_block;
  }
  return block_size + 1;
}

struct free_blocks find_free_blocks(void *firstbyte,
                                    struct mem_control_block *current) {
  struct free_blocks blocks = {NULL, NULL};
  // if there are no free blocks, return NULL
  if (current == NULL) {
    return blocks;
  }
  // if current is the previous free block, return that
  else if ((void *)current < firstbyte &&
           (current->next == NULL || firstbyte < (void *)current->next)) {
    blocks.prev = current;
    blocks.next = current->next;
    return blocks;
  }
  // if there is no previous free block
  else if (firstbyte < (void *)current) {
    blocks.prev = NULL;
    blocks.next = current;
    return blocks;
  }
  // if there are many preceding free blocks, keep iterating
  else {
    return find_free_blocks(firstbyte, current->next);
  }
}

int free_block_exists_before(struct free_blocks blocks) {
  return blocks.prev != NULL;
}

int free_block_exists_after(struct free_blocks blocks) {
  return blocks.next != NULL;
}

int free_blocks_exist(struct free_blocks blocks) {
  return free_block_exists_before(blocks) || free_block_exists_after(blocks);
}

int is_previous_free_block_adjacent(struct free_blocks blocks,
                                    long *size_of_block) {
  void *last_address_of_prev = (void *)blocks.prev + blocks.prev->size;
  return last_address_of_prev == size_of_block;
}

int is_next_free_block_adjacent(struct free_blocks blocks,
                                long *size_of_block) {
  return blocks.next == (void *)size_of_block + *size_of_block;
}

void merge_consecutive_blocks(long *size_of_block,
                              struct free_blocks blocks,
                              struct mem_control_block *m) {
  m->size = *size_of_block + blocks.next->size;
  m->next = blocks.next->next;
}

void myfree(void *firstbyte) {
  long *size_of_block = (long *)(firstbyte - sizeof(long));

  struct free_blocks blocks = find_free_blocks(firstbyte, free_list_start);

  // pointer needed if we do not have to merge with preceding free block
  struct mem_control_block *m = (struct mem_control_block *)size_of_block;

  // there are no free blocks
  if (!free_blocks_exist(blocks)) {
    m->size = *size_of_block;
    m->next = NULL;

    free_list_start = m;
  }
  // there is no free block before the block we want to free
  else if (!free_block_exists_before(blocks)) {
    // check if there's a free block immediately after
    if (is_next_free_block_adjacent(blocks, size_of_block)) {
      // merge the two blocks
      merge_consecutive_blocks(size_of_block, blocks, m);
    } else {
      m->size = *size_of_block;
      m->next = blocks.next;
    }

    free_list_start = m;
  }
  // there is no free block after the block we want to free
  else if (!free_block_exists_after(blocks)) {
    // check if there's a free block immediately before
    if (is_previous_free_block_adjacent(blocks, size_of_block)) {
      // add size of freed block to preceding free block
      blocks.prev->size = blocks.prev->size + *size_of_block;
    } else {
      m->size = *size_of_block;
      m->next = NULL;

      blocks.prev->next = m;
    }
  }
  /* there are at least one free block before and after the 
  * block we want to free */
  else {
    // check if there's a free block immediately on both sides
    if (is_previous_free_block_adjacent(blocks, size_of_block) &&
        is_next_free_block_adjacent(blocks, size_of_block)) {
      // add freed block and succeeding free block to preceding free block
      blocks.prev->size =
          blocks.prev->size + *size_of_block + blocks.next->size;
      blocks.prev->next = blocks.next->next;
    }
    // check if there's a free block immediately after
    else if (is_next_free_block_adjacent(blocks, size_of_block)) {
      merge_consecutive_blocks(size_of_block, blocks, m);
      blocks.prev->next = m;
    }
    // check if there's a free block immediately before
    else if (is_previous_free_block_adjacent(blocks, size_of_block)) {
      blocks.prev->size = blocks.prev->size + *size_of_block;
    }
    // no block on either side that needs merging
    else {
      m->size = *size_of_block;
      m->next = blocks.next;

      blocks.prev->next = m;
    }
  }
}

// for debugging
void print_free_list(struct mem_control_block *current) {
  if (current == NULL) {
    printf("\nEnd of free list\n");
  } else {
    printf("\nFree block address: %p, size: %d, next: %p", current,
           current->size, current->next);
    print_free_list(current->next);
  }
}

int main(int argc, char **argv) {
  /* ------------------------------------------ */
  /* Rounding tests*/
  printf("ROUNDING FOR MEMORY ALIGNMENT:\n");

  printf("Rounds values below 8 to 16: ");
  if (calc_rounded(1) == 16 && calc_rounded(8) == 16) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Rounds values above 14 to the next multiple of 8: ");
  if (calc_rounded(15) == 24 && calc_rounded(40) == 48) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */
  /* Simple allocation tests */
  printf("\nSIMPLE ALLOCATION:\n");

  void *result = mymalloc(0);

  printf("Returns NULL if asked to allocate 0 bytes: ");
  if (result == NULL) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("When nothing is allocated, the first mem_control_block has length "
         "64*1024 minus the first mem_control_block: ");
  if (free_list_start->size == MEM_SIZE) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("When one block is allocated, our free block spans the rest of the "
         "memory: ");

  long numbytes = 40;
  result = mymalloc(numbytes);

  if (free_list_start->size == MEM_SIZE - calc_rounded(numbytes)) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("When one block is allocated, mymalloc returns managed_memory_start + "
         "sizeof(long): ");

  if (result == managed_memory_start + sizeof(long)) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("When one block is allocated, the free block struct is located after "
         "the allocated block: ");

  if (free_list_start == managed_memory_start + calc_rounded(numbytes)) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Sets the first 8 bytes of the allocated block to the size of the "
         "block: ");

  long *block_size = managed_memory_start;
  if (*block_size == calc_rounded(numbytes)) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Returns NULL if we try to allocate more than the memory holds: ");

  long numbytes2 = 64 * 1024 - 1;
  result = mymalloc(numbytes2);

  if (result == NULL) {
    printf("YES\n");
  } else {
    printf("NO %p\n", result);
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can allocate a block that takes all remaining memory: ");

  long numbytes3 = MEM_SIZE - calc_rounded(numbytes) - sizeof(long);
  result = mymalloc(numbytes3);

  if (result == managed_memory_start + sizeof(long) + calc_rounded(numbytes) &&
      free_list_start == NULL) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf(
      "Can allocate a block that creates a free block smaller than 16 bytes: ");

  mymalloc_init(); // reset state
  mymalloc(numbytes);
  long numbytes4 = MEM_SIZE - calc_rounded(numbytes) - sizeof(long) - 15;
  result = mymalloc(numbytes4);

  if (result == managed_memory_start + sizeof(long) + calc_rounded(numbytes) &&
      free_list_start == NULL) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can allocate a 16 byte block when asked to create smaller blocks: ");

  mymalloc_init();
  result = mymalloc(1);
  void *result2 = mymalloc(7);

  if (result2 == managed_memory_start + 16 + sizeof(long) &&
      (void *)free_list_start == managed_memory_start + 32) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can allocate a second block, without overwriting the first block: ");

  mymalloc_init();
  mymalloc(numbytes);
  long numbytes5 = 64;
  result = mymalloc(numbytes5);

  if (result == managed_memory_start + sizeof(long) + calc_rounded(numbytes)) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */
  /* Tests for find_suitable_block */
  printf("\nFIND SUITABLE BLOCK:\n");

  printf("Returns first suitable block when that block is passed to the "
         "function: ");

  struct free_blocks first_suitable_result =
      find_suitable_block(40, NULL, free_list_start);
  if ((void *)first_suitable_result.next == free_list_start &&
      (void *)first_suitable_result.prev == NULL) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Returns NULL if there are no suitable blocks: ");

  mymalloc_init();
  long numbytes6 = 64 * 1024 - 50;
  mymalloc(numbytes6);
  first_suitable_result = find_suitable_block(100, NULL, free_list_start);

  if ((void *)first_suitable_result.next == NULL &&
      (void *)first_suitable_result.prev == NULL) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Recursively finds suitable block and it's predecessor: ");

  mymalloc_init();
  mymalloc(20);
  mymalloc(30);
  result = mymalloc(40);
  mymalloc(30);
  result2 = mymalloc(30);
  void *result3 = mymalloc(50);
  myfree(result);
  myfree(result2);

  first_suitable_result = find_suitable_block(49, NULL, free_list_start);

  if ((void *)first_suitable_result.next == result3 + 56 &&
      (void *)first_suitable_result.prev == result2 - 8) {
    printf("YES\n");
  } else {
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

  if (result - sizeof(long) == free_list_start) {
    printf("YES\n");
  } else {
    printf("NO\n");
    printf("%p, %p\n", free_list_start, result);
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can merge a freed block with a succeeding free block: ");

  mymalloc_init();
  mymalloc(40);
  result = mymalloc(20);
  myfree(result);

  if (result - sizeof(long) == free_list_start) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can merge a freed block with a preceding free block: ");

  mymalloc_init();
  mymalloc(numbytes);
  void *first_to_free = mymalloc(20);
  void *second_to_free = mymalloc(30);
  mymalloc(MEM_SIZE - numbytes - 50 - 4 * sizeof(long));
  myfree(first_to_free);
  myfree(second_to_free);

  if (first_to_free - sizeof(long) == free_list_start) {
    printf("YES\n");
  } else {
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
  mymalloc(MEM_SIZE - 110 - 4 * sizeof(long));
  myfree(first_to_free);
  myfree(second_to_free);
  myfree(third_to_free);

  if (first_to_free - sizeof(long) == free_list_start &&
      free_list_start->next == NULL) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */
  /* More extensive tests that combines allocation and deallocation */

  printf("\nCOMPOUND TESTS:\n");

  printf("Can skip a free space and allocate in next when the first free space "
         "is not large enough: ");

  mymalloc_init();
  mymalloc(40);
  void *block_to_free = mymalloc(20);
  mymalloc(30);

  myfree(block_to_free);

  result = mymalloc(25);

  if (result == managed_memory_start + 120 + sizeof(long) &&
      (void *)free_list_start < result) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }

  /* ------------------------------------------ */

  printf("Can allocate something in first free space when first free space is "
         "large enough: ");

  mymalloc_init();
  mymalloc(40);
  block_to_free = mymalloc(20);
  mymalloc(30);

  myfree(block_to_free);

  result = mymalloc(20);

  if (result == managed_memory_start + 48 + sizeof(long) &&
      result < (void *)free_list_start) {
    printf("YES\n");
  } else {
    printf("NO\n");
    exit(EXIT_FAILURE);
  }
}
