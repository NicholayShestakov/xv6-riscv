#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int child_pid = fork();
  if (child_pid == -1) {
    printf("Child processes was not created.\n");
    exit(-1);
  } else if (child_pid == 0) {
    if (pause(100) == -1) {
      exit(-1);
    }
    exit(1);
  }
  printf("Parent pid: %d\n", getpid());
  printf("Child pid: %d\n", child_pid);
  if (kill(child_pid) == -1) {
    printf("Process with given pid does not exists.\n");
    exit(-1);
  }
  int child_status;
  int wait_res = wait(&child_status);
  if (wait_res == -1) {
    printf("Child processes was not exists.\n");
    exit(-1);
  }
  printf("Ended process pid: %d\n", wait_res);
  printf("Ended process exit code: %d\n", child_status);
  exit(0);
}
