#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

int main()
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t items_count = page_size / sizeof(unsigned int);

    unsigned int *buffer = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (buffer == MAP_FAILED) { perror("mmap"); exit(1); }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }

    if (pid == 0)
    {
        // === Писатель ===
        unsigned int counter = 0;
        while (1)
        {
            for (size_t i = 0; i < items_count; i++)
                buffer[i] = counter++;
        }
    }
    else
    {
        // === Читатель ===
        unsigned int expected = 0;
        long errors = 0;

        for (long iter = 0; iter < 10000; iter++)
        {
            for (size_t i = 0; i < items_count; i++)
            {
                if (buffer[i] != expected)
                {
                    printf("!!! Сбой: ожидалось %u, получено %u\n", expected, buffer[i]);
                    expected = buffer[i];
                    errors++;
                }
                expected++;
            }
        }

        printf("Готово. Сбоев обнаружено: %ld\n", errors);
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }

    munmap(buffer, page_size);
    return 0;
}
