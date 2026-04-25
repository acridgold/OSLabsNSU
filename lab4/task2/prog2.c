#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

void recursive_stack_growth(int depth) {
    char buffer[4096];

    memset(buffer, 0, sizeof(buffer));

    if (depth % 5 == 0) {
        printf("  [Stack] Глубина рекурсии: %d, адрес буфера: %p\n", depth, (void*)buffer);
        sleep(1);
    }

    if (depth < 50) {
        recursive_stack_growth(depth + 1);
    }
}

int main() {
    int pid = getpid();
    long page_size = sysconf(_SC_PAGESIZE);
    printf("=== PID процесса: %d ===\n", pid);
    printf("Мониторинг: watch -d -n 1 cat /proc/%d/maps\n", pid);
    printf("Ожидание 10 секунд для начала мониторинга...\n");
    sleep(10);

    // 1. Стек
    printf("\n1. Растем по стеку (рекурсия)...\n");
    sleep(5);
    recursive_stack_growth(1);
    printf("Стек вырос. Проверьте [stack] в картах памяти. Пауза 5с.\n");
    sleep(5);

    // 2. Куча
    printf("\n2. Выделение памяти на куче (5 итераций по 1МБ)...\n");
    sleep(5);
    char *heap_chunks[5];
    for (int i = 0; i < 5; i++) {
        heap_chunks[i] = malloc(1024 * 1024);
        if (heap_chunks[i]) {
            memset(heap_chunks[i], 1, 1024 * 1024);
            printf("  [Heap] Выделен 1 МБ, адрес: %p\n", (void*)heap_chunks[i]);
        }
        sleep(5);
    }
    printf("Куча выросла. Проверь [heap]. Пауза 5с.\n");
    sleep(5);

    // 3. Освобождение
    printf("\n3. Освобождаем память кучи...\n");
    sleep(5);
    for (int i = 0; i < 5; i++) {
        free(heap_chunks[i]);
    }
    printf("Память освобождена. Пауза 5с.\n");
    sleep(5);

    // 4. mmap
    printf("\n4. Создаем анонимный регион (10 страниц)...\n");
    sleep(5);
    size_t region_size = 10 * page_size;
    void *mmap_addr = mmap(NULL, region_size, PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mmap_addr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    printf("  [mmap] Регион создан по адресу: %p\n", mmap_addr);
    sleep(5);

    // 5. munmap части региона
    printf("\n5. Отсоединяем страницы с 4 по 6 в регионе (дырка)...\n");
    sleep(5);
    munmap((char*)mmap_addr + 3 * page_size, 3 * page_size);
    printf("Проверьте map. Пауза 5с.\n");
    sleep(5);

    // 6. Права доступа и Segfault
    printf("\n6. Изменяем права доступа на PROT_NONE...\n");
    mprotect(mmap_addr, page_size, PROT_NONE);
    printf("Записи...\n");
    fflush(stdout);

    *(int*)mmap_addr = 123;

    return 0;
}