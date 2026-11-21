#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 3 || argc > 3) {
    printf("Incorrect argument count\n");
    exit(1);
  }

  int fd = open(argv[2], O_RDONLY);
  if (fd < 0) {
    printf("Open error\n");
    exit(1);
  }

  int readed = 0;
  char buf[512];
  if ((readed = read(fd, buf, atoi(argv[1]))) < 0) {
    printf("Read error\n");
    exit(1);
  }
  for (int i = 0; i < readed; ++i)
    printf("%x ", buf[i]);
  printf("\n");
}