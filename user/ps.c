#include "kernel/types.h"
#include "user/user.h"

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
    (void)plist;
  }

  if (count < 0) {
    exit(-1);
  }

  for (int i = 0; i < count; ++i) {
    struct procinfo *pi = &plist[i];
    printf("%d %s %d %d\n", pi->pid, pi->name, pi->state, pi->ppid);
  }
  exit(0);
}
