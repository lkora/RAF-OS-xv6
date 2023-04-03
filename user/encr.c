#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"

char buf[512];

void encr_usage(void)
{
    printf("Usage: encr [OPTIONS] [FILE]...\n"
            "Encrypt one or more files using Caesar cipher.\n"
            "Options:\n"
            "  -h, --help: Show this help message and exit\n"
            "  -a, --encrypt-all: Encrypt all non-encrypted files in the working directory\n");
}

void encrypt_file(char *path)
{
    int fd;

    if ((fd = open(path, O_RDWR)) < 0)
    {
        printf("encr: cannot open %s\n", path);
        return;
    }

    if (encr(fd) < 0)
    {
        printf("encr: failed to encrypt %s\n", path);
        close(fd);
        return;
    }

    close(fd);
}

void encrypt_all(void)
{
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(".", O_RDONLY)) < 0)
    {
        printf("encr: cannot open .\n");
        return;
    }

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0 || de.name[0] == '.')
            continue;
        if (stat(de.name, &st) < 0)
        {
            printf("encr: cannot stat %s\n", de.name);
            continue;
        }
        if (st.type != T_FILE)
            continue;
        encrypt_file(de.name);
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    int i;
    int encrypt_all_flag = 0;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            encr_usage();
            exit();
        }
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--encrypt-all") == 0)
        {
            encrypt_all_flag = 1;
        }
        else if (argv[i][0] == '-')
        {
            printf("encr: unknown option %s\n", argv[i]);
            encr_usage();
            exit();
        }
        else
        {
            break;
        }
    }

    if (i == argc && !encrypt_all_flag)
    {
        encr_usage();
        exit();
    }

    if (encrypt_all_flag)
    {
        encrypt_all();
        exit();
    }

    for (; i < argc; i++)
    {
        encrypt_file(argv[i]);
    }

    exit();
}