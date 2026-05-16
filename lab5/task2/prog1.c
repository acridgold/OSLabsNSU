#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int global_var = 100;

int main()
{
    int local_var = 200;

    printf("PID перед fork(): %d\n", getpid());
    printf("Адрес global_var: %p, значение: %d\n",
           &global_var, global_var);
    printf("Адрес local_var:  %p, значение: %d\n",
           &local_var, local_var);

    printf("Мониторинг: watch -d -n 1 ps %d\n", getpid());
    sleep(10);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        printf("\n=== Дочерний процесс ===\n");
        printf("Child: PID = %d\n", getpid());
        printf("Child: родительский PID = %d\n", getppid());
        printf("Мониторинг: watch -d -n 1 ps %d\n", getpid());
        sleep(10);

        printf("Child: адрес global_var = %p, значение = %d\n",
               &global_var, global_var);
        printf("Child: адрес local_var  = %p, значение = %d\n",
               &local_var, local_var);

        global_var = 777;
        local_var = 888;
        printf("Child: произошли изменения\n");
        printf("Child: адрес global_var = %p, значение = %d\n",
               &global_var, global_var);
        printf("Child: адрес local_var  = %p, значение = %d\n",
               &local_var, local_var);

        sleep(2);

        printf("Child: завершаюсь с кодом 5.\n");
        exit(5);
    }
    printf("\n=== Родительский процесс ===\n");
    printf("Parent: мой PID = %d\n", getpid());
    printf("Parent: PID ребенка = %d\n", pid);

    printf("Parent: адрес global_var = %p, значение = %d\n",
           &global_var, global_var);
    printf("Parent: адрес local_var  = %p, значение = %d\n",
           &local_var, local_var);

    printf("Parent: засыпаю на 30 сек. "
        "Можно изучить procfs.\n");

    while(1) {
        sleep(10);
    }

    int status;
    printf("Parent: жду завершения дочернего...\n");
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
    {
        printf("Parent: Дочерний процесс завершился "
            "нормально.\n");
        printf("Parent: Код завершения: %d\n",
               WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        printf("Parent: Дочерний процесс убит "
               "сигналом: %d\n", WTERMSIG(status));
    }
    else if (WIFSTOPPED(status))
    {
        printf("Parent: Дочерний процесс остановлен "
               "сигналом: %d\n", WSTOPSIG(status));
    }

    printf("PID %d: завершение работы.\n", getpid());
    return 0;
}

// Можно посмотреть инфу через `ps PID` или `top` (сверху будут зомби)