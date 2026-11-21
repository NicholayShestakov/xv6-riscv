#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char *
date()
{
  uint64 *nanoseconds = read();
}