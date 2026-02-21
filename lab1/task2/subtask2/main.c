#include <stdio.h>

void hello_from_dynamic_lib();

int main()
{
    printf("Hello, World!\n");
    hello_from_dynamic_lib();
    return 0;
}