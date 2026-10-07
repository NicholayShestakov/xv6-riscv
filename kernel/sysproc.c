#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_ps_listinfo(void)
{
  uint64 plist;
  argaddr(0, &plist);
  if (plist == 0) {
    return -1;
  }
  int lim;
  argint(1, &lim);
  if (lim < 0) {
    return -1;
  }

  struct proc *proc = get_proc_list();
  int counter = 0;
  struct spinlock *wait_lock = get_wait_lock();

  for (int i = 0; i < NPROC; ++i) {
    struct proc *p = &proc[i];
    acquire(wait_lock);
    if (p->parent != 0) {
      acquire(&(p->parent->lock));
    }
    acquire(&(p->lock));
    if (p->state != USED && p->state != UNUSED) {
      if (counter < lim) {
        struct procinfo pinfo;
        pinfo.pid = p->pid;
        safestrcpy(pinfo.name, p->name, sizeof(p->name));
        pinfo.state = p->state;
        if (p->parent == 0) {
          pinfo.ppid = 0;
        } else {
          pinfo.ppid = p->parent->pid;
        }
        if (copyout(myproc()->pagetable, myproc()->sz,
                    plist + (counter * sizeof(struct procinfo)),
                    (char *)(&pinfo), sizeof(struct procinfo))) {
          release(&(p->lock));
          if (p->parent != 0) {
            release(&(p->parent->lock));
          }
          release(wait_lock);
          return -1;
        }
      }
      ++counter;
    }
    release(&(p->lock));
    if (p->parent != 0) {
      release(&(p->parent->lock));
    }
    release(wait_lock);
  }

  return counter;
}
