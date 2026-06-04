#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH   "/tmp/echo.sock"
#define BUF_SIZE    4096

int main(int argc, char* argv[])
{
    int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr = {.sun_family = AF_LOCAL};
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("connect (сервер запущен?)");
        close(fd);
        return 1;
    }

    printf("[client pid=%d] connected to '%s'\n", getpid(), SOCK_PATH);

    char buf[BUF_SIZE];

    // Одна посылка
    if (argc > 1)
    {
        buf[0] = '\0';
        for (int i = 1; i < argc; i++)
        {
            strncat(buf, argv[i], sizeof(buf) - strlen(buf) - 2);
            if (i < argc - 1) strncat(buf, " ", sizeof(buf) - strlen(buf) - 1);
        }
        size_t len = strlen(buf);

        printf("[client] send: \"%s\"\n", buf);
        if (send(fd, buf, len, 0) < 0)
        {
            perror("send");
            goto done;
        }

        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n > 0)
        {
            buf[n] = '\0';
            printf("[client] echo: \"%s\"\n", buf);
        }
        goto done;
    }

    // Интерактивный режим
    printf("[client] интерактивный режим (Ctrl+D для выхода)\n");
    printf("> ");
    fflush(stdout);

    while (fgets(buf, sizeof(buf), stdin))
    {
        size_t len = strlen(buf);

        if (len == 0 || buf[0] == '\n')
        {
            printf("> ");
            fflush(stdout);
            continue;
        }

        if (send(fd, buf, len, 0) < 0)
        {
            perror("send");
            break;
        }

        const ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0)
        {
            if (n < 0) perror("recv");
            else printf("[client] сервер закрыл соединение\n");
            break;
        }
        buf[n] = '\0';

        if (buf[n - 1] == '\n') buf[n - 1] = '\0';
        printf("echo: %s\n> ", buf);
        fflush(stdout);
    }

done:
    close(fd);
    printf("\n[client] disconnected\n");
    return 0;
}
