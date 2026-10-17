#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
print_chars(int pid, int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        for (int j = 0; arg[j] != '\0'; j++) {
            printf("%d: arg %d, char '%c'\n", pid, i, arg[j]);
        }
    }
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(2, "usage: mutextes arg1 arg2 arg3 ...\n");
        exit(1);
    }

    printf("Test_1 without mutex\n");
    int pid = fork();
    if (pid < 0 ) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        print_chars(getpid(), argc, argv);
        exit(0);
    }

    print_chars(getpid(), argc, argv);
    wait(0);

    printf("Test_2 with mutex\n");
    int m = mutex();
    if (m < 0) {
        fprintf(2, "mutex failed\n");
        exit(1);
    }

    pid = fork();

    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        int my_pid = getpid();
        for (int i = 1; i < argc; i++) {
            for (int j = 0; argv[i][j] != '\0'; j++) {
                mutex_lock(m);
                printf("%d: arg %d, char '%c'\n", my_pid, i, argv[i][j]);
                mutex_unlock(m);
            }
        }
        close(m);
        exit(0);
    }

    int my_pid = getpid();
    for (int i = 1; i < argc; i++) {
        for (int j = 0; argv[i][j] != '\0'; j++) {
            mutex_lock(m);
            printf("%d: arg %d, char '%c'\n", my_pid, i, argv[i][j]);
            mutex_unlock(m);
        }
    }
    close(m);
    wait(0);
    
    exit(0);


}