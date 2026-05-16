#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if (pid == 0)
    {
        pid_t grandson = fork();

        if (grandson == 0)
        {
            sleep(5);
            printf("Сын: Мой новый родитель (PPID) = %d\n", getppid());
            exit(0);
        }
        printf("Отец: Завершаюсь, становлюсь зомби для Деда...\n");
        exit(0);
    }
    printf("Дед: Мой PID = %d. Я буду спать 10 сек.\n", getpid());
    printf("Проверь статус Отца: watch -d -n 1 ps %d\n", pid);
    sleep(10);
    wait(NULL);
    printf("Дед: Убрал зомби-отца.\n");
    return 0;
}
