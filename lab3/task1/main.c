#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>
#include <stdlib.h>

#define BUF_SIZE 1024

void reverse(char *s) {
    int i, j;
    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        char temp = s[i];
        s[i] = s[j];
        s[j] = temp;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <путь_к_каталогу>\n", argv[0]);
        return 1;
    }

    char *old_path = strdup(argv[1]);

    size_t len = strlen(old_path);
    if (len > 0 && old_path[len - 1] == '/') {
        old_path[len - 1] = '\0';
    }

    char *base = basename(old_path);
    char new_dir_name[256];
    strcpy(new_dir_name, base);
    reverse(new_dir_name);

    if (mkdir(new_dir_name, 0777) == -1) {
        perror("Ошибка при создании каталога");
    }

    DIR *dir = opendir(old_path);
    if (!dir) {
        perror("Не удалось открыть исходный каталог");
        free(old_path);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG) continue;

        char path_in[1024], path_out[1024], file_rev[256];

        snprintf(path_in, sizeof(path_in), "%s/%s", old_path, entry->d_name);

        strcpy(file_rev, entry->d_name);
        reverse(file_rev);
        snprintf(path_out, sizeof(path_out), "%s/%s", new_dir_name, file_rev);

        FILE *f_in = fopen(path_in, "rb");
        FILE *f_out = fopen(path_out, "wb");

        if (f_in && f_out) {
            fseek(f_in, 0, SEEK_END); // todo: зачем
            long pos = ftell(f_in);
            char buffer[BUF_SIZE]; //uns char

            while (pos > 0) {
                long chunk = pos < BUF_SIZE ? pos : BUF_SIZE;
                fseek(f_in, pos - chunk, SEEK_SET);
                size_t read_bytes = fread(buffer, 1, chunk, f_in);
                if (read_bytes != chunk) {
                    break;
                }

                for (long i = 0; i < chunk / 2; i++) {
                    char tmp = buffer[i];
                    buffer[i] = buffer[chunk - 1 - i];
                    buffer[chunk - 1 - i] = tmp;
                }
                fwrite(buffer, 1, chunk, f_out);
                pos -= chunk;
            }

        }

        if (f_in) fclose(f_in);
        if (f_out) fclose(f_out);
    }

    closedir(dir);
    free(old_path);
    //printf("Готово! Проверьте каталог: %s\n", new_dir_name);
    return 0;
}