#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_ids() {
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}

// Какой общий алгоритм работы с UID в файликах
// Как распределяются права на выполнение, если пользователь в нескольких группах
// Как вмешивается в это SUID
// Какие есть проверки на группы

int main() {
    const char *path = "secret.txt";
    FILE *file;
    char buffer[256];

    print_ids();

    file = fopen(path, "r");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return 1;
    }

    printf("Содержимое файла: ");
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }

    fclose(file);
    return 0;
}