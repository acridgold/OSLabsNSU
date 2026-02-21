#include <stdio.h>
#include <dlfcn.h>

int main()
{
    void* lib = dlopen("./libhello.so", RTLD_LAZY); // При RTLD_LAZY функция вызывается только по требованию
    if (!lib)
    {
        fprintf(stderr, "Error: %s\n", dlerror());
        return 1;
    }

    printf("Hello, World!\n");

    void (*hello)() = dlsym(lib, "hello_from_dyn_runtime_lib");
    if (!hello)
    {
        fprintf(stderr, "Error: %s\n", dlerror());
        return 1;
    }

    hello();

    dlclose(lib);
    return 0;
}