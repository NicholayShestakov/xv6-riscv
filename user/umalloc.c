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
  Header *bp, *p, *prevp;

  bp = (Header *)ap - 1;
  int array_index = 0;
  for (int i = 8; i < bp->s.size && array_index < FREEP_ARR_SIZE; i *= 2) {
    ++array_index;
  }

  for (p = freep_array[array_index]; !(bp > p && bp < p->s.ptr);
       prevp = p, p = p->s.ptr)
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;

  if (bp + bp->s.size == p->s.ptr) {
    while (bp + bp->s.size == p->s.ptr) {
      prevp->s.ptr = p->s.ptr;
      bp->s.size += p->s.ptr->s.size;
      bp->s.ptr = p->s.ptr->s.ptr;

      for (p = freep_array[array_index++];
           !(bp > p && bp < p->s.ptr) && array_index < FREEP_ARR_SIZE - 1;
           prevp = p, p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
          break;
    }
  }
  bp->s.ptr = p->s.ptr;
  freep_array[array_index] = p;
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
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  int array_index = 0;
  for (int i = 8; i < nunits && array_index < FREEP_ARR_SIZE; i *= 2) {
    ++array_index;
  }

  if ((prevp = freep_array[array_index]) == 0) {
    base.s.ptr = freep_array[array_index] = prevp = &base;
    base.s.size = 0;
  }
  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        if (array_index > 0 && p->s.size <= (8 << (array_index - 1))) {
          prevp->s.ptr = p->s.ptr;
          for (Header *p_down = freep_array[array_index - 1];
               !(p > p_down && p < p_down->s.ptr); p_down = p_down->s.ptr)
            if (p_down >= p_down->s.ptr && (p > p_down || p < p_down->s.ptr))
              break;
        }
        p += p->s.size;
        p->s.size = nunits;
      }
      freep_array[array_index] = prevp;
      return (void *)(p + 1);
    }
    if (p == freep_array[array_index])
      if (array_index < FREEP_ARR_SIZE - 1) {
        p = freep_array[array_index++];
      } else if ((p = morecore(nunits, array_index)) == 0)
        return 0;
  }
}
