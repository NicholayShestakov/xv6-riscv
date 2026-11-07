#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

#define FREEP_ARR_SIZE 10 // 8 - 4096+
static Header *freep_array[FREEP_ARR_SIZE] = {0};

static Header base;

void
free(void *ap)
{
  // printf("free: free called with pointer %p\n", ap);
  Header *bp, *p;

  bp = (Header *)ap - 1;
  // printf("free: size of element is %d\n", bp->s.size);
  int array_index = 0;
  for (int i = 8; i < bp->s.size && array_index < FREEP_ARR_SIZE; i *= 2) {
    ++array_index;
  }
  // printf("free: array index is %d\n", array_index);

  while (array_index < FREEP_ARR_SIZE) {
    for (p = freep_array[array_index]; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
      if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
        break;
    // printf("free: previous pointer is %p\n", p);
    if (bp + bp->s.size == p->s.ptr && array_index < FREEP_ARR_SIZE - 1) {
      // printf("free: merging with %p\n", p);
      bp->s.size += p->s.ptr->s.size;
      p->s.ptr = p->s.ptr->s.ptr;
      ++array_index;
      // printf("free: next ring with array index %d\n", array_index);
    } else {
      // printf("free: successfully freed %p memory\n", ap);
      bp->s.ptr = p->s.ptr;
      p->s.ptr = bp;
      freep_array[array_index] = p;
      break;
    }
  }
  // char *edge = sbrk(0);
  // printf("free:\n");
  // printf("  Data segment edge - %p\n", edge);
  // printf("  Blocks:\n");
  // int flag = 0, flag2 = 0;
  // for (char *i = 0; i < edge; ++i) {
  //   for (int j = 0; j < FREEP_ARR_SIZE; ++j) {
  //     for (p = freep_array[j];; p = p->s.ptr) {
  //       if (p == 0) {
  //         break;
  //       }
  //       if ((char *)p == i) {
  //         printf("    %p - occuped\n", p);
  //         flag2 = 1;
  //         flag = 1;
  //         break;
  //       }
  //       if (p->s.ptr == freep_array[j]) {
  //         break;
  //       }
  //     }
  //     if (flag) {
  //       break;
  //     }
  //   }
  //   if (!flag && !flag2) {
  //     flag2 = 1;
  //     printf("    %p - free\n", i);
  //   }
  // }
}

static Header *
morecore(uint nu, int idx)
{
  char *p;
  Header *hp;

  if (nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if (p == SBRK_ERROR)
    return 0;
  hp = (Header *)p;
  hp->s.size = nu;
  free((void *)(hp + 1));
  return freep_array[idx];
}

void *
malloc(uint nbytes)
{
  // printf("malloc: malloc %d bytes starts\n", nbytes);
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  // printf("malloc: allocating %d units\n", nunits);
  int array_index = 0;
  for (int i = 8; i < nunits && array_index < FREEP_ARR_SIZE; i *= 2) {
    ++array_index;
  }
  // printf("malloc: array index is %d\n", array_index);

  if ((prevp = freep_array[array_index]) == 0) {
    // printf("malloc: current array ring is empty\n");
    base.s.ptr = freep_array[array_index] = prevp = &base;
    base.s.size = 0;
  }
  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      // printf("malloc: adress %p with size %d founded\n", p->s.ptr, p->s.size);
      if (p->s.size == nunits) {
        // printf("malloc: sizes equivalent\n");
        prevp->s.ptr = p->s.ptr;
      } else {
        // printf("malloc: sizes not equivalent. Getting part of element size\n");
        p->s.size -= nunits;
        if (array_index > 0 && p->s.size <= (8 << (array_index - 1))) {
          // printf(
          //   "malloc: element is too small to this ring after borrowing part of it. Going to the previous ring\n");
          Header *p_down = freep_array[array_index - 1];
          prevp->s.ptr = p->s.ptr;
          for (Header *p_down = freep_array[array_index - 1];
               !(p > p_down && p < p_down->s.ptr); p_down = p_down->s.ptr)
            if (p_down >= p_down->s.ptr && (p > p_down || p < p_down->s.ptr))
              break;
          p->s.ptr = p_down->s.ptr;
          // printf("malloc: element successfully downed into previous ring\n");
        }
        p += p->s.size;
        p->s.size = nunits;
      }
      freep_array[array_index] = prevp;
      // printf("malloc: successfully allocated memory at %p adress\n", p->s.ptr);
      // char *edge = sbrk(0);
      // printf("malloc:\n");
      // printf("  Data segment edge - %p\n", edge);
      // printf("  Blocks:\n");
      // int flag = 0, flag2 = 0;
      // for (char *i = 0; i < edge; ++i) {
      //   for (int j = 0; j < FREEP_ARR_SIZE; ++j) {
      //     for (p = freep_array[j];; p = p->s.ptr) {
      //       if (p == 0) {
      //         break;
      //       }
      //       if ((char *)p == i) {
      //         printf("    %p - occuped\n", p);
      //         flag2 = 1;
      //         flag = 1;
      //         break;
      //       }
      //       if (p->s.ptr == freep_array[j]) {
      //         break;
      //       }
      //     }
      //     if (flag) {
      //       break;
      //     }
      //   }
      //   if (!flag && !flag2) {
      //     flag2 = 1;
      //     printf("    %p - free\n", i);
      //   }
      // }
      return (void *)(p + 1);
    }
    if (p == freep_array[array_index]) {
      if (array_index < FREEP_ARR_SIZE - 1) {
        p = freep_array[array_index++];
        if ((prevp = freep_array[array_index]) == 0) {
          // printf("malloc: current array ring is empty\n");
          base.s.ptr = freep_array[array_index] = prevp = &base;
          base.s.size = 0;
        }
        // printf(
        //   "malloc: in this ring not founded correct size element. Going to the next ring %d\n",
        //   array_index);
      } else if ((p = morecore(nunits, array_index)) == 0) {
        // printf("malloc: new memory allocated incorrect\n");
        return 0;
      }
    }
  }
}
