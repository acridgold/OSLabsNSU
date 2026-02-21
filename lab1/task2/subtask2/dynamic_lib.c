#include <stdio.h>

int a = 9999;

int* get_a_ptr() {
    return &a;
}

void hello_from_dynamic_lib()
{
    printf("Hello from dynamic!\n");
}

int big_array[1000] = {1, 2, 3, 4, 5};

void access_array(int index) {
    if (index >= 0 && index < 1000) {
        printf("big_array[%d] = %d\n", index, big_array[index]);
    }
}