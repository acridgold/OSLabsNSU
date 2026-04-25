#include <stdio.h>
#include <stddef.h>
#include <sys/mman.h>
#include <string.h>

#define HEAP_SIZE (1024 * 1024)

typedef struct block
{
    size_t size;
    int free;
    struct block* next;
} block_t;

static block_t* heap_start = NULL;

void heap_init(void)
{
    heap_start = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (heap_start == MAP_FAILED)
    {
        perror("mmap");
        heap_start = NULL;
        return;
    }

    heap_start->size = HEAP_SIZE - sizeof(block_t);
    heap_start->free = 1;
    heap_start->next = NULL;
}

void* my_malloc(size_t size)
{
    if (!heap_start || size == 0)
        return NULL;

    block_t* cur = heap_start;
    while (cur)
    {
        if (cur->free && cur->size >= size)
        {
            if (cur->size >= size + sizeof(block_t) + 1)
            {
                block_t* rest = (block_t*)((char*)(cur + 1) + size);
                rest->size = cur->size - size - sizeof(block_t);
                rest->free = 1;
                rest->next = cur->next;

                cur->size = size;
                cur->next = rest;
            }
            cur->free = 0;
            return cur + 1;
        }
        cur = cur->next;
    }

    return NULL;
}

void my_free(void* ptr)
{
    if (!ptr) return;

    char* heap_end = (char*)heap_start + HEAP_SIZE;
    if ((char*)ptr < (char*)(heap_start + 1) || (char*)ptr >= heap_end)
    {
        fprintf(stderr, "my_free: указатель %p вне кучи [%p, %p)\n",
                ptr, (void*)(heap_start + 1), (void*)heap_end);
        return;
    }

    block_t* cur = heap_start;
    while (cur)
    {
        if ((void*)(cur + 1) == ptr)
            break;
        cur = cur->next;
    }
    if (!cur)
    {
        fprintf(stderr, "my_free: указатель %p не является началом блока\n", ptr);
        return;
    }

    block_t* blk = (block_t*)ptr - 1;
    blk->free = 1;

    cur = heap_start;
    while (cur && cur->next)
    {
        block_t* expected = (block_t*)((char*)(cur + 1) + cur->size);
        if (cur->next != expected)
        {
            fprintf(stderr, "heap corruption: cur->next=%p expected=%p\n",
                    (void*)cur->next, (void*)expected);
            return; // сделать аборт()
        }

        if (cur->free && cur->next->free)
        {
            cur->size += sizeof(block_t) + cur->next->size;
            cur->next = cur->next->next;
        }
        else
        {
            cur = cur->next;
        }
    }
}

void heap_dump(void)
{
    printf("\n--- heap dump ---\n");
    block_t* cur = heap_start;
    int i = 0;
    while (cur)
    {
        printf("  [%d] addr=%p  size=%-6zu  %s\n",
               i++, (void*)(cur + 1), cur->size,
               cur->free ? "FREE" : "USED");
        cur = cur->next;
    }
    printf("-----------------\n");
}

int main(void)
{
    heap_init();
    printf("Куча инициализирована (%d байт).\n", HEAP_SIZE);
    heap_dump();

    void* a = my_malloc(100);
    void* b = my_malloc(200);
    void* c = my_malloc(50);
    printf("\nВыделено: a=%p  b=%p  c=%p\n", a, b, c);
    heap_dump();

    my_free(b);
    printf("\nОсвободили b.\n");
    heap_dump();

    void* d = my_malloc(64);
    printf("\nВыделили d=%p.\n", d);
    heap_dump();

    my_free(a);
    my_free(c);
    my_free(d);
    printf("\nОсвободили всё.\n");
    heap_dump();

    munmap(heap_start, HEAP_SIZE);
    return 0;
}
