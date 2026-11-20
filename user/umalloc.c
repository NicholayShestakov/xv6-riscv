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

static Header base[FREEP_ARR_SIZE];

Header *
pop(Header *prev, Header *p)
{
  prev->s.ptr = p->s.ptr;
  return p;
}

void
push(Header *prev, Header *p)
{
  p->s.ptr = prev->s.ptr;
  prev->s.ptr = p;
}

int
merge(Header *p)
{
  if (p + p->s.size == p->s.ptr) {
    p->s.size += p->s.ptr->s.size;
    p->s.ptr = p->s.ptr->s.ptr;
    return 0;
  }
  return 1;
}

static int is_heap_start_filled = 0;
static char *heap_start;

void
debug_info()
{
  char *edge = sbrk(0);
  printf("  Heap start: %p\n", heap_start);
  printf("  Data segment edge - %p\n", edge);
  printf("  Blocks:\n");
  int occuped_size = 0;
  int current_free_size = 0;
  int current_free_ring = 0;
  int current_occuped_size = 0;
  int flag = 0;
  for (char *i = heap_start; i < edge;) {
    for (int idx = 0; idx < FREEP_ARR_SIZE; ++idx) {
      for (Header *p = freep_array[idx];; p = p->s.ptr) {
        if (p == 0)
          break;
        if ((char *)p == i && p->s.size != 0) {
          flag = 1;
          current_free_size = p->s.size;
          current_free_ring = idx;
          break;
        }
        if (p->s.ptr == freep_array[idx])
          break;
      }
      if (flag)
        break;
    }
    if (flag) {
      if (current_occuped_size > 0) {
        printf("    %p - size %ld, occuped\n", i - current_occuped_size,
               current_occuped_size * sizeof(Header));
        occuped_size += current_occuped_size;
        current_occuped_size = 0;
      }
      printf("    %p - size %ld, free, ring %d\n", i,
             current_free_size * sizeof(Header), current_free_ring);
      i += current_free_size * sizeof(Header);
      flag = 0;
    } else {
      ++current_occuped_size;
      i += sizeof(Header);

      if (i == edge) {
        printf("    %p - size %ld, occuped\n", i - current_occuped_size,
               current_occuped_size * sizeof(Header));
        occuped_size += current_occuped_size;
      }
    }
  }
  printf("  Total allocated memory: %ld\n", (long)(edge - heap_start));
  printf("  Total occuped memory: %ld\n",
         (long)(occuped_size * sizeof(Header)));
  printf("  Total free memory: %ld\n",
         (long)(edge - heap_start - occuped_size * sizeof(Header)));
}

void
free(void *ap)
{
  Header *bp, *p;

  bp = (Header *)ap - 1;
  int array_index = 0;
  for (int i = 8; i < bp->s.size && array_index < FREEP_ARR_SIZE - 1; i *= 2)
    ++array_index;

  while (array_index < FREEP_ARR_SIZE) {
    if (freep_array[array_index] == 0) {
      base[array_index].s.ptr = freep_array[array_index] = &base[array_index];
      base[array_index].s.size = 0;
    }
    for (p = freep_array[array_index]; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
      if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
        break;
    push(p, bp);
    freep_array[array_index] = p;
    merge(bp);
    if (bp->s.size <= 8 << array_index)
      break;
    if (++array_index < FREEP_ARR_SIZE)
      pop(p, bp);
  }
  printf("free:\n");
  debug_info();
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
  if (!is_heap_start_filled) {
    is_heap_start_filled = 1;
    heap_start = p;
  }
  hp = (Header *)p;
  hp->s.size = nu;
  printf("morecore ");
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
  for (int i = 8; i < nunits && array_index < FREEP_ARR_SIZE - 1; i *= 2) {
    ++array_index;
  }

  if ((prevp = freep_array[array_index]) == 0) {
    base[array_index].s.ptr = freep_array[array_index] = prevp =
      &base[array_index];
    base[array_index].s.size = 0;
  }
  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        if (array_index > 0 && p->s.size <= (8 << (array_index - 1))) {
          pop(prevp, p);
          int down_array_index = 0;
          for (int i = 8;
               i < p->s.size && down_array_index < FREEP_ARR_SIZE - 1; i *= 2)
            ++down_array_index;
          Header *p_down;
          while (down_array_index < FREEP_ARR_SIZE) {
            if (freep_array[down_array_index] == 0) {
              base[down_array_index].s.ptr = freep_array[down_array_index] =
                &base[down_array_index];
              base[down_array_index].s.size = 0;
            }
            for (p_down = freep_array[down_array_index];
                 !(p > p_down && p < p_down->s.ptr); p_down = p_down->s.ptr)
              if (p_down >= p_down->s.ptr && (p > p_down || p < p_down->s.ptr))
                break;
            push(p_down, p);
            freep_array[down_array_index] = p_down;
            merge(p);
            if (p->s.size <= 8 << down_array_index)
              break;
            if (++down_array_index < FREEP_ARR_SIZE)
              pop(p_down, p);
          }
        }
        p += p->s.size;
        p->s.size = nunits;
      }
      freep_array[array_index] = prevp;
      printf("malloc (successfull):\n");
      debug_info();
      return (void *)(p + 1);
    }
    if (p == freep_array[array_index]) {
      if (array_index < FREEP_ARR_SIZE - 1) {
        p = freep_array[++array_index];
        if ((prevp = freep_array[array_index]) == 0) {
          base[array_index].s.ptr = freep_array[array_index] = prevp =
            &base[array_index];
          base[array_index].s.size = 0;
          p = prevp;
        }
      } else if ((p = morecore(nunits, array_index)) == 0) {
        printf("malloc (failed):\n");
        debug_info();
        return 0;
      }
    }
  }
}
