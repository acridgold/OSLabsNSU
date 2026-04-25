#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096
#define PAGEMAP_ENTRY_SIZE 8

typedef struct {
    uint64_t pfn : 55; // адрес в RAM
    unsigned int soft_dirty : 1; // менялась ли страница
    unsigned int exclusive : 1; // принадлежит ли страница только вашему процессу
    unsigned int reserved : 4; // Зарезервировано ядром
    unsigned int file_shared : 1; // связана ли страница с файлом на диске или это анонимная память
    unsigned int swapped : 1; // данные в Swap
    unsigned int present : 1; // данные физически в RAM
} PagemapEntry;

void print_entry(uint64_t vaddr, uint64_t raw_entry) {
    PagemapEntry *entry = (PagemapEntry *)&raw_entry;
    printf("0x%012lx | ", vaddr);

    if (entry->present) {
        printf("RAM  | PFN: 0x%07lx | Физический: 0x%012lx",
                (uint64_t)entry->pfn, (uint64_t)entry->pfn * PAGE_SIZE);
        if (entry->exclusive) printf(" | [EXCL]");
        if (entry->soft_dirty) printf(" | [DIRTY]");
    } else if (entry->swapped) {
        printf("SWAP | Своп-индекс: %llu", (unsigned long long)entry->pfn);
    } else {
        printf("EMPTY| Не выделена (PFN: 0)");
    }
    printf("\n");
}

void process_region(int fd, uint64_t start, uint64_t end) {
    uint64_t num_pages = (end - start) / PAGE_SIZE;
    uint64_t raw_entry;

    off_t offset = (start / PAGE_SIZE) * PAGEMAP_ENTRY_SIZE;
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) return;

    for (uint64_t i = 0; i < num_pages; i++) { // RAM or SWAP
        if (read(fd, &raw_entry, PAGEMAP_ENTRY_SIZE) == PAGEMAP_ENTRY_SIZE) {
            if ((raw_entry >> 63) & 1 || (raw_entry >> 62) & 1) {
                print_entry(start + (i * PAGE_SIZE), raw_entry);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (geteuid() != 0) {
        printf("Для чтения PFN нужны права root.\n");
    }

    pid_t pid = (argc > 1) ? atoi(argv[1]) : getpid();
    char path[256];

    printf("=== PID: %d ===\n", pid);

    snprintf(path, sizeof(path), "/proc/%d/pagemap", pid);
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Ошибка открытия pagemap");
        return 1;
    }

    snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    FILE *maps = fopen(path, "r");
    if (!maps) {
        perror("Ошибка открытия maps");
        close(fd);
        return 1;
    }

    char line[512];
    while (fgets(line, sizeof(line), maps)) {
        uint64_t start, end;
        if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
            process_region(fd, start, end);
        }
    }

    if (pid == getpid()) {
        printf("\n--- Демонстрация mmap (2 страницы) ---\n");
        size_t size = PAGE_SIZE * 2;
        void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (ptr != MAP_FAILED) {
            printf("Адрес: %p. До записи (Empty):\n", ptr);
            process_region(fd, (uintptr_t)ptr, (uintptr_t)ptr + size);

            memset(ptr, 'A', size);
            printf("После записи (RAM allocated):\n");
            process_region(fd, (uintptr_t)ptr, (uintptr_t)ptr + size);

            munmap(ptr, size);
        }
    }

    fclose(maps);
    close(fd);
    return 0;
}