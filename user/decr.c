#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/decr.h"

char buf[512];

void decr_usage(void) {
    fprintf(1, "Usage: decr [OPTIONS] [FILE]...\n"
            "Decrypt one or more files using Caesar cipher.\n"
            "Options:\n"
            "  -h, --help: Show this help message and exit\n"
            "  -a, --decrypt-all: Decrypt all encrypted files in the working directory\n");
}

void decrypt_file(char *path) {
    int fd;

    if ((fd = open(path, O_RDWR)) < 0) {
        fprintf(2, "decr: cannot open %s\n", path);
        return;
    }

    int decr_status = decr(fd);
    if (decr_status < 0) {
        fprintf(2, "encr: failed to decrypt %s with code: %d\n", path, decr_status);
        close(fd);
        return;
    }

    close(fd);
}

void decrypt_all(void)
{
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(".", O_RDONLY)) < 0) {
        fprintf(2, "decr: cannot open .\n");
        return;
    }

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0 || de.name[0] == '.')
            continue;
        if (stat(de.name, &st) < 0) {
            fprintf(2, "decr: cannot stat %s\n", de.name);
            continue;
        }
        if (st.type != T_FILE)
            continue;
        decrypt_file(de.name);
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    int i;
    int decrypt_all_flag = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            decr_usage();
            exit();
        }
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--decrypt-all") == 0) {
            decrypt_all_flag = 1;
        }
        else if (argv[i][0] == '-') {
            fprintf(1, "decr: unknown option %s\n", argv[i]);
            decr_usage();
            exit();
        }
        else {
            break;
        }
    }

    if (i == argc && !decrypt_all_flag) {
        decr_usage();
        exit();
    }

    if (decrypt_all_flag) {
        decrypt_all();
        exit();
    }

    for (; i < argc; i++) {
        decrypt_file(argv[i]);
    }

    exit();
}