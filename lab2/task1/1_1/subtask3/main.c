#include <unistd.h>

ssize_t new_write(int fd, const void *buf, size_t count) {
    return syscall(1, fd, buf, count);
}

int main() {
    new_write(1, "Hello from my_write!\n", 21);
    return 0;
}