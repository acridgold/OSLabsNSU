#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sched.h>

#define STACK_SIZE  (1024 * 1024)
#define STACK_FILE  "stack_dump.x"
#define MAX_DEPTH   10

static void recursive_hello(const int depth)
{
    // Массив на стеке
    volatile char msg[] = "hello world";
    printf("[child] depth=%d  &depth=%p msg @ %p = \"%s\"\n",
           depth, (void*)&depth, (void*)msg, msg);
    if (depth > 0)
    {
        recursive_hello(depth - 1);
    }
}

static int child_entry(void* arg)
{
    (void)arg; // Требует аргументы, не используем
    printf("[child] pid=%d  started\n", getpid());

    recursive_hello(MAX_DEPTH);

    printf("[child] done, exiting\n");
    _exit(0);
}

int main(void)
{
    int fd = open(STACK_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    if (ftruncate(fd, STACK_SIZE) < 0)
    {
        perror("ftruncate");
        return 1;
    }

    printf("[parent] stack file '%s' created, size=%d bytes\n",
           STACK_FILE, STACK_SIZE);

    void* stack_base = mmap(
        NULL,
        STACK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );
    if (stack_base == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    printf("[parent] stack mapped: base=%p  top=%p\n",
           stack_base,
           (char*)stack_base + STACK_SIZE);

    void* stack_top = (char*)stack_base + STACK_SIZE;
    stack_top = (void*)((uintptr_t)stack_top & ~(uintptr_t)15);

    pid_t child_pid = clone(
        child_entry,
        stack_top,
        SIGCHLD,
        NULL
    );

    if (child_pid < 0)
    {
        perror("clone");
        return 1;
    }

    printf("[parent] clone() -> child pid=%d\n", child_pid);


    int status;
    waitpid(child_pid, &status, 0);

    if (msync(stack_base, STACK_SIZE, MS_SYNC) < 0) {
        perror("msync");
    }

    if (WIFEXITED(status))
        printf("[parent] child exited with code %d\n", WEXITSTATUS(status));

    munmap(stack_base, STACK_SIZE);
    close(fd);

    printf("[parent] stack dumped to '%s', analyse with:\n"
           "  strings %s\n"
           "  xxd %s | grep -A1 'hell'\n",
           STACK_FILE, STACK_FILE, STACK_FILE);

    return 0;
}
