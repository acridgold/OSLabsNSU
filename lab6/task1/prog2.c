#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>

int main() {
    long page_size = sysconf(_SC_PAGESIZE);
    size_t items_count = page_size / sizeof(unsigned int);

    unsigned int *buffer = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (buffer == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Маска сигналов
    sigset_t wait_mask;
    sigemptyset(&wait_mask);
    sigaddset(&wait_mask, SIGUSR1); // Для синхронизации
    sigaddset(&wait_mask, SIGINT);  // Для корректного выхода
    sigaddset(&wait_mask, SIGUSR2); // Уведомление о смерти партнера

    // Блокируем сигналы чтобы не провоцировать ложного поведения
    if (sigprocmask(SIG_BLOCK, &wait_mask, NULL) == -1) {
        perror("sigprocmask");
        exit(1);
    }

    pid_t parent_pid = getpid();
    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("fork");
        exit(1);
    }

    if (child_pid == 0) {
        // ПИСАТЕЛЬ
        unsigned int counter = 0;
        int sig;
        printf("[Писатель %d] Запущен. Начинаю заполнение...\n", getpid());

        while (1) {
            for (size_t i = 0; i < items_count; i++) {
                buffer[i] = counter++;
            }

            // Сообщаем "Буфер заполнен"
            kill(parent_pid, SIGUSR1);

            // Ждем ответный сигнал
            sigwait(&wait_mask, &sig);

            if (sig == SIGINT) {
                printf("\n[Писатель] Получен SIGINT. Уведомляю читателя и выхожу.\n");
                kill(parent_pid, SIGUSR2);
                break;
            }
            if (sig == SIGUSR2) {
                printf("\n[Писатель] Читатель завершился. Я тоже ухожу.\n");
                break;
            }
        }
    } else {
        // ЧИТАТЕЛЬ
        unsigned int expected = 0;
        int sig;
        printf("[Читатель %d] Запущен. Жду данные...\n", getpid());

        while (1) {
            // Ждем сигнал от Писателя
            sigwait(&wait_mask, &sig);

            if (sig == SIGINT) {
                printf("\n[Читатель] Получен SIGINT. Уведомляю писателя и выхожу.\n");
                kill(child_pid, SIGUSR2);
                break;
            }
            if (sig == SIGUSR2) {
                printf("\n[Читатель] Писатель завершился. Завершаю работу.\n");
                break;
            }

            // Проверка
            for (size_t i = 0; i < items_count; i++) {
                if (buffer[i] != expected) {
                    printf("\n!!! СБОЙ: Ожидалось %u, в буфере %u\n", expected, buffer[i]);
                    expected = buffer[i];
                }
                expected++;
            }

            // Сообщаем "Буфер свободен"
            kill(child_pid, SIGUSR1);
        }

        // Ждем завершения ребенка
        waitpid(child_pid, NULL, 0);
    }

    munmap(buffer, page_size);
    printf("Процесс %d: Ресурсы освобождены.\n", getpid());
    return 0;
}