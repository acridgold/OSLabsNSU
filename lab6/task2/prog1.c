#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // === Читатель ===
        close(fd[1]);

        char buf[256];
        ssize_t n;
        while ((n = read(fd[0], buf, sizeof(buf))) > 0)
        {
            for (ssize_t i = 0; i < n; i++)
                buf[i] = toupper((unsigned char)buf[i]);
            write(STDOUT_FILENO, buf, n);
        }

        close(fd[0]);
        exit(0);
    }
    // === Писатель ===
    close(fd[0]);

    char line[256];
    while (fgets(line, sizeof(line), stdin) != NULL)
        write(fd[1], line, strlen(line));

    close(fd[1]);
    wait(NULL);

    return 0;
}
