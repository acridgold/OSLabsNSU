#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH   "/tmp/echo.sock"
#define BACKLOG     8
#define BUF_SIZE    4096

static volatile sig_atomic_t g_stop = 0;

static void stop_handler(const int sig) {
    (void)sig;
    g_stop = 1;
}

static void serve_client(const int client_fd)
{
    char buf[BUF_SIZE];
    ssize_t n;

    printf("[child pid=%d] serving client on fd=%d\n", getpid(), client_fd);

    while ((n = recv(client_fd, buf, sizeof(buf), 0)) > 0)
    {
        buf[n] = '\0';
        printf("[child pid=%d] got %zd bytes: \"%.*s\"\n",
               getpid(), n, (int)n, buf);

        ssize_t sent = 0;
        while (sent < n)
        {
            const ssize_t w = send(client_fd, buf + sent, n - sent, 0);
            if (w < 0)
            {
                perror("send");
                goto done;
            }
            sent += w;
        }
    }

    if (n < 0)
        perror("recv");
    else
        printf("[child pid=%d] client disconnected\n", getpid());

done:
    close(client_fd);
}

int main(void)
{
    struct sigaction sa_stop = {
        .sa_handler = stop_handler,
        .sa_flags = 0
    };
    sigemptyset(&sa_stop.sa_mask);
    sigaction(SIGINT,  &sa_stop, NULL);
    sigaction(SIGTERM, &sa_stop, NULL);

    int server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }
    unlink(SOCK_PATH);


    struct sockaddr_un addr = {.sun_family = AF_LOCAL};
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("listen");
        return 1;
    }

    printf("[server] listening on '%s'\n", SOCK_PATH);

    while (true)
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0)
        {
            if (errno == EINTR)
            {
                if (g_stop) break;
                continue;
            }
            perror("accept");
            break;
        }
        printf("[server] accepted client -> fd=%d\n", client_fd);

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0)
        {
            // Дочерний процесс
            close(server_fd);
            serve_client(client_fd);
            exit(0);
        }

        // Родительский процесс
        close(client_fd);
        printf("[server] forked child pid=%d for client\n", pid);
    }

    close(server_fd);
    unlink(SOCK_PATH);
    return 0;
}
