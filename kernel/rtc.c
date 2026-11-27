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

// get address of rtc registers
#define Reg(reg) ((volatile uint32 *)(RTC + (reg)))
// read register value
#define ReadReg(reg) (*(Reg(reg)))

struct spinlock lock;

int
read_registers(int user_dst, uint64 dst, int n)
{
  if (sizeof(uint64) != n) {
    return -1;
  }
  acquire(&lock);
  uint32 younger_word = ReadReg(0);
  uint32 older_word = ReadReg(4);
  release(&lock);
  uint64 rtcbuf = ((uint64)older_word << 32) | younger_word;

  // user_dst = 0 - kernel memory, 1 - user memory
  if (either_copyout(user_dst, dst, &rtcbuf, n) == -1) {
    return -1;
  }
  return 8;
}

int
write_registers(int user_dst, uint64 dst, int n)
{
  return -1;
}

void
rtcinit()
{
  initlock(&lock, "rtc");
  devsw[RTC_MAJOR].write = write_registers;
  devsw[RTC_MAJOR].read = read_registers;
}