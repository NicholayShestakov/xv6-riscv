#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"

#include "file.h"

int
mutexalloc(struct file **f)
{
  printk("mutexalloc: alloc begin\n");
  if ((*f = filealloc()) == 0) {
    printk("mutexalloc: filealloc error\n");
    return -1;
  }
  printk("mutexalloc: file allocated\n");
  struct sleeplock *sl = kalloc();
  if (sl == 0) {
    printk("mutexalloc: kalloc error. closing file\n");
    fileclose(*f);
    return -1;
  }
  printk("mutexalloc: sleeplock allocated\n");
  initsleeplock(sl, "mutex");
  printk("mutexalloc: sleeplock initialized\n");

  (*f)->type = FD_MUTEX;
  (*f)->readable = 0;
  (*f)->writable = 0;
  (*f)->mutex = sl;
  printk("mutexalloc: file content written\n");

  return 0;
}

void
mutexclose(struct sleeplock *m)
{
  printk("mutexclose: freeing mutex\n");
  kfree(m);
  printk("mutexclose: mutex freed\n");
}
