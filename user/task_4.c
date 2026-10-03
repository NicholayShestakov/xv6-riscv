/*
 * Данный файл является домашним заданием не для конкретной ОС.
 * Просто не хочется создавать ещё один репозиторий, поэтому добавил сюда.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
  int pipefd[2];

  if (pipe(pipefd) < 0) {
    perror("Pipe error");
    exit(1);
  }

  int pid = fork();

  if (pid < 0) {
    perror("Fork error");
    exit(1);
  }

  if (pid == 0) {
    close(pipefd[1]);
    char buf[64];
    int read_status;
    while ((read_status = read(pipefd[0], buf, sizeof(buf))) != 0) {
      if (read_status < 0) {
        perror("Read error");
        exit(1);
      }
      int write_status = write(STDOUT_FILENO, buf, read_status);
      if (write_status < 0) {
        perror("Child write error");
        exit(1);
      }
    }
    close(pipefd[0]);
  } else {
    close(pipefd[0]);
    for (int i = 1; i < argc; i++) {
      int len = strlen(argv[i]);
      int written = 0;
      while (written < len) {
        int write_status = write(pipefd[1], argv[i] + written, len - written);
        if (write_status < 0) {
          close(pipefd[1]);
          perror("Write error");
          exit(1);
        }
        written += write_status;
      }
      if (write(pipefd[1], "\n", 1) < 0) {
        close(pipefd[1]);
        perror("Write error");
        exit(1);
      }
    }

    close(pipefd[1]);
    int status;
    wait(&status);
    if (status != 0) {
      perror("Child process error");
      exit(1);
    }
  }

  exit(0);
}
