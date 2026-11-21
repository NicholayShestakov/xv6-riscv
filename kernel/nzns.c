#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

struct spinlock nznslock;

static int written_bytes = 0;

int
nznswrite(int user_src, uint64 src, int n, short minor)
{
  switch (minor) {
  case NZNS_NULL:
    return n;
  case NZNS_NULLSTAT:
    acquire(&nznslock);
    written_bytes += n;
    release(&nznslock);
    return n;
  default:
    return -1;
  }
}

int
nznsread(int user_dst, uint64 dst, int n, short minor)
{
  char zero_byte = 0;
  switch (minor) {
  case NZNS_NULL:
    return 0;
  case NZNS_ZERO:
    for (int i = 0; i < n; ++i) {
      acquire(&nznslock);
      if (either_copyout(user_dst, dst + i, &zero_byte, 1) != -1) {
        release(&nznslock);
        return n;
      }
    }
    release(&nznslock);
  case NZNS_NULLSTAT:
    if (n == sizeof(uint64)) {
      acquire(&nznslock);
      if (either_copyout(user_dst, dst, &written_bytes, 1) != -1) {
        release(&nznslock);
        return sizeof(uint64);
      }
    }
    release(&nznslock);
  default:
    return -1;
  }
}

void
nznsinit(void)
{
  initlock(&nznslock, "nznslock");

  devsw[NZNS].read = nznsread;
  devsw[NZNS].write = nznswrite;
}
