#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("PID: %d\n", getpid());
    printf("Мониторинг: watch -n 1 cat /proc/%d/maps\n", pid);

    sleep(5);

    char *args[] = {argv[0], NULL};
    execv(argv[0], args);

    printf("Hello world\n");
    return 0;
}