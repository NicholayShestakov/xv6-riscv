#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define PID_COL    "PID"
#define NAME_COL   "NAME"
#define STATE_COL  "STATE"
#define PARENT_COL "PARENT"

int
intlen(int n)
{
  int count = 0;
  if (n <= 0) {
    count = 1;
  }
  while (n != 0) {
    n /= 10;
    ++count;
  }
  return count;
}

// Prints integer with space offset to the right.
// If integer length is bigger than offset, then prints full integer.
void
printint_with_offset(int n, uint offset)
{
  for (int i = 0; i < offset - intlen(n); ++i) {
    printf(" ");
  }
  printf("%d", n);
}

// Prints string with space offset to the right.
// If string length is bigger than offset, then prints full string.
void
printstr_with_offset(char *s, uint offset)
{
  for (int i = 0; i < offset - strlen(s); ++i) {
    printf(" ");
  }
  printf("%s", s);
}

int
max(int a, int b)
{
  return a > b ? a : b;
}

int
main(int argc, char *argv[])
{
  int size = 10;
  struct procinfo *plist = malloc(size * sizeof(struct procinfo));
  int count;
  while ((count = ps_listinfo(plist, size)) > size) {
    size *= 2;
    free(plist);
    plist = malloc(size * sizeof(struct procinfo));
  }

  if (count < 0) {
    free(plist);
    exit(-1);
  }

  char *states[] = {[UNUSED] = "unused",     [USED] = "used",
                    [SLEEPING] = "sleeping", [RUNNABLE] = "runnable",
                    [RUNNING] = "running",   [ZOMBIE] = "zombie"};

  int max_state_len = 0;
  for (int i = 0; i < sizeof(states) / sizeof(states[0]); ++i) {
    max_state_len = max(max_state_len, strlen(states[i]));
  }

  printstr_with_offset(PID_COL, max(intlen(NPROC), strlen(PID_COL)));
  printstr_with_offset(NAME_COL, sizeof(((struct procinfo *)0)->name) + 1);
  printstr_with_offset(STATE_COL, max(max_state_len, strlen(STATE_COL)) + 1);
  printstr_with_offset(PARENT_COL, max(intlen(NPROC), strlen(PARENT_COL)) + 1);
  printf("\n");

  for (int i = 0; i < count; ++i) {
    struct procinfo *pi = &plist[i];
    printint_with_offset(pi->pid, max(intlen(NPROC), strlen(PID_COL)));
    printstr_with_offset(pi->name, sizeof(((struct procinfo *)0)->name) + 1);
    printstr_with_offset(states[pi->state],
                         max(max_state_len, strlen(STATE_COL)) + 1);
    printint_with_offset(pi->ppid, max(intlen(NPROC), strlen(PARENT_COL)) + 1);
    printf("\n");
  }
  free(plist);
  exit(0);
}
