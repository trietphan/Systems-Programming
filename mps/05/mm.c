/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define BLK_HDR_SIZE (ALIGN(sizeof(blk_hdr_t)))
#define NUM_SIZE 5
#define PROLOGUE_SIZE BLK_HDR_SIZE * (NUM_SIZE + 2)

/* header and footer block */
#define HF_BLK_SIZE 2*BLK_HDR_SIZE

typedef struct header blk_hdr_t;

struct header {
  size_t size;
  blk_hdr_t *next;
  blk_hdr_t *prev;
};

blk_hdr_t *find_fit(size_t size);
blk_hdr_t *split(blk_hdr_t *bp, size_t size);
blk_hdr_t *get_footer(blk_hdr_t *bp);
void remove_from_list(blk_hdr_t *bp);
void add_to_list(blk_hdr_t *bp);
void coalesce(blk_hdr_t *bp);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  blk_hdr_t *prologue = mem_sbrk(PROLOGUE_SIZE);
  prologue->size = (PROLOGUE_SIZE) | 1;
  blk_hdr_t *prologue_footer = get_footer(prologue);
  prologue->next = prologue;
  prologue->prev = prologue;
  prologue_footer->size = (PROLOGUE_SIZE) | 1;
   /* fill the prologue block */
  size_t min_class_size[] = { HF_BLK_SIZE,
                              HF_BLK_SIZE+64,
                              HF_BLK_SIZE+128,
                              HF_BLK_SIZE+256,
                              HF_BLK_SIZE+512 };
  int i = 0;
  blk_hdr_t *bp = (blk_hdr_t *)((char *)mem_heap_lo() + BLK_HDR_SIZE);
  while (bp < prologue_footer) {
    bp->next = bp;
    bp->prev = bp;
    bp->size = min_class_size[i++];
    bp = (blk_hdr_t *)((char *)bp + BLK_HDR_SIZE);
  }
  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  int blksize = ALIGN(HF_BLK_SIZE + size);
  blk_hdr_t *bp = find_fit(blksize);
  if (bp == NULL) {
    bp = mem_sbrk(blksize);
    if ((long)bp == -1)
      return NULL;
    else {
      bp->size = blksize | 1;
      blk_hdr_t *bp_footer = get_footer(bp);
      bp_footer->size = blksize | 1;
    }
  } else {
    bp = split(bp, blksize);
    remove_from_list(bp);
  }
  return (char *)bp + BLK_HDR_SIZE;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  blk_hdr_t *bp = ptr - BLK_HDR_SIZE;
  add_to_list(bp);
  coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  void *oldptr = ptr;
  void *newptr;
  size_t copySize;

  newptr = mm_malloc(size);
  if (newptr == NULL)
    return NULL;
  copySize = *(size_t *)((char *)oldptr - BLK_HDR_SIZE);
  if (size < copySize)
    copySize = size;
  memcpy(newptr, oldptr, copySize);
  mm_free(oldptr);
  return newptr;
}


blk_hdr_t *get_footer(blk_hdr_t *bp) {
  return (blk_hdr_t *)((char *)bp + (bp->size & ~1L) - BLK_HDR_SIZE);
}
 
/* split - split a block into two if there is enough space */ 
blk_hdr_t *split(blk_hdr_t *bp, size_t size) {
  size_t remaining = bp->size - size;
  blk_hdr_t *p;

  if (remaining < ((blk_hdr_t *)((char *)mem_heap_lo() + BLK_HDR_SIZE))->size) {
    return bp;
  } else {
    bp->size = size & ~1L; 
    blk_hdr_t *footer_bp = get_footer(bp);
    footer_bp->size = size & ~1L;
    p = (blk_hdr_t *)((char *)bp + size);
    p->size = remaining & ~1L;
    blk_hdr_t *footer_p = get_footer(p);
    footer_p->size = remaining & ~1L;  
    /* insert new block into the list */
    remove_from_list(bp);
    add_to_list(bp);
    add_to_list(p);
    return bp;
  }
}

 /* find_fit - tries to find an empty block from a correct free list based on its size */
 blk_hdr_t *find_fit(size_t size)
 {
   blk_hdr_t *bp = mem_heap_lo(),
     *footer = get_footer(bp),
     *head = bp;

   for (++head;
        !((size > head->size && size < (head+1)->size) || head+1 == footer);
        head++);

   for (bp = head->next;
        bp != head && bp->size < size;
        bp = bp->next);

   if (bp != head) {
     return bp;
   } else {
      return NULL;
   }
}

/* remove_from_list remove element from free list */
 void remove_from_list(blk_hdr_t *bp) {
   bp->size |= 1;
   get_footer(bp)->size |= 1;
   bp->prev->next = bp->next;
   bp->next->prev = bp->prev;
 }

 /* add_to_list add element to a correct free list  */
void add_to_list(blk_hdr_t *bp) {
  blk_hdr_t *head = mem_heap_lo(),
    *footer = get_footer(head);

  bp->size &= ~1;
  get_footer(bp)->size &= ~1;

  for (++head; 
       !((bp->size > head->size && bp->size < (head+1)->size) || head+1 == footer);
       head++);

  bp->next = head->next;
  bp->prev = head;
  head->next = head->next->prev = bp;
}

/*
 * coalesce - merge free blocks that are physically next to each other
 */
void coalesce(blk_hdr_t *bp) {
  // assuming that *bp, *next and *prev are all in the free list before we run this function
  blk_hdr_t *next = NULL,
    *prev = NULL,
    *footer = NULL;

  if (bp != mem_heap_hi() - bp->size + 1) {
    next = (blk_hdr_t *)((char *)bp + (bp->size & ~1L)); // TODO: add epilogue block for this edge-case
  }

  blk_hdr_t *prev_footer = (blk_hdr_t *)((char *)bp - BLK_HDR_SIZE);
  prev = (blk_hdr_t *)((char *)bp - (prev_footer->size & ~1L));

  int next_alloc, prev_alloc;

  if (next) {
    next_alloc = next->size & 1;
  } else {
    next_alloc = 1;
  }

  if (prev) {
    prev_alloc = prev->size & 1;
  } else {
    prev_alloc = 1;
  }

  if (prev_alloc && next_alloc) {
    return;
  } else if (prev_alloc && !next_alloc) {
    bp->size += next->size;
    // get the footer of the next block and set its new size
    footer = get_footer(bp);
    footer->size = bp->size;
    // Remove next from ll
    remove_from_list(bp);
    remove_from_list(next);
    add_to_list(bp);
    return;
  } else if (!prev_alloc && next_alloc) {
    prev->size += bp->size;
    // get footer of prev and set it to its new size
    footer = get_footer(prev);
    footer->size = prev->size;
    remove_from_list(bp);
    remove_from_list(prev);
    add_to_list(prev);
    return;
  } else {
    // both next and prev are free
    prev->size += (bp->size + next->size);
    // and same footer deal again
    footer = get_footer(prev);
    footer->size = prev->size;
    // remove next and bp from ll
    remove_from_list(prev);
    remove_from_list(bp);
    remove_from_list(next);
    add_to_list(prev);
    return;
  }
}
