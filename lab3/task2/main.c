#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>

void print_stat(char* path)
{
    struct stat st;
    if (lstat(path, &st) == 0)
    {
        printf("Файл: %s\n", path);
        printf("Права доступа: %o\n", st.st_mode & 0777);
        printf("Количество жестких ссылок: %ld\n", (long)st.st_nlink);
    }
    else
    {
        perror("Ошибка stat");
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Использование: %s <аргумент1> [аргумент2]\n", argv[0]);
        return 1;
    }

    char* cmd = basename(argv[0]);

    if (strcmp(cmd, "cmd_mkdir") == 0) return mkdir(argv[1], 0755);

    if (strcmp(cmd, "cmd_ls") == 0)
    {
        DIR* d = opendir(argv[1]);
        if (!d) return 1;
        struct dirent* en;
        while ((en = readdir(d))) printf("%s\n", en->d_name);
        return closedir(d);
    }

    if (strcmp(cmd, "cmd_rmdir") == 0) return rmdir(argv[1]);

    if (strcmp(cmd, "cmd_touch") == 0)
    {
        FILE* f = fopen(argv[1], "w");
        if (f) return fclose(f);
        return 1;
    }

    if (strcmp(cmd, "cmd_cat") == 0)
    {
        FILE* f = fopen(argv[1], "r");
        if (!f) return 1;
        char c;
        while ((c = fgetc(f)) != EOF) putchar(c);
        return fclose(f);
    }

    if (strcmp(cmd, "cmd_rm") == 0) return unlink(argv[1]);

    if (strcmp(cmd, "cmd_ln_s") == 0) return symlink(argv[1], argv[2]);

    if (strcmp(cmd, "cmd_readlink") == 0)
    {
        char buf[1024];
        ssize_t len = readlink(argv[1], buf, sizeof(buf) - 1);
        if (len != -1)
        {
            buf[len] = '\0';
            printf("%s\n", buf);
            return 0;
        }
        return 1;
    }

    if (strcmp(cmd, "cmd_cat_link") == 0)
    {
        FILE* f = fopen(argv[1], "r");
        if (!f) return 1;
        char c;
        while ((c = fgetc(f)) != EOF) putchar(c);
        return fclose(f);
    }

    if (strcmp(cmd, "cmd_rm_s") == 0) return unlink(argv[1]);

    if (strcmp(cmd, "cmd_ln") == 0) return link(argv[1], argv[2]);

    if (strcmp(cmd, "cmd_rm_h") == 0) return unlink(argv[1]);

    if (strcmp(cmd, "cmd_stat") == 0)
    {
        print_stat(argv[1]);
        return 0;
    }

    if (strcmp(cmd, "cmd_chmod") == 0) return chmod(argv[1], strtol(argv[2], NULL, 8));

    printf("Неизвестная команда: %s\n", cmd);
    return 1;
}
