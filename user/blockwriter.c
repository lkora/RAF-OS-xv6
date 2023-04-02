#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"

#define BLOCK_SIZE 150

void usage()
{
    printf("\nUse this program to create a big file filled with a-z characters.\n"
           "Default filename: long.txt\n"
           "Default blocks: 150\n"
           "Usage: blockwriter [OPTION]...\n"
           "\nCommand line options:\n"
           "  -h,  --help: Show help prompt.\n"
           "  -o,  --output-file: Set output filename .\n"
           "  -b,  --blocks: Number of blocks to write.\n");
}

int main(int argc, char *argv[]) {
    char *filename = "long.txt";
    int blocks = 1;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage();
            exit();
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-file") == 0) {
            if (i + 1 >= argc) {
                printf("Error: missing argument for %s\n", argv[i]);
                usage();
                exit();
            }
            filename = argv[++i];
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--blocks") == 0) {
            if (i + 1 >= argc) {
                printf("Error: missing argument for %s\n", argv[i]);
                usage();
                exit();
            }
            blocks = atoi(argv[++i]);
        } else {
            printf("Error: unknown option %s\n", argv[i]);
            usage();
            exit();
        }
    }

    int fd = open(filename, O_CREATE | O_WRONLY);
    if (fd < 0) {
        printf("Error: failed to open %s\n", filename);
        exit();
    }

    char buf[BLOCK_SIZE];
    for (i = 0; i < BLOCK_SIZE; i++) {
        buf[i] = 'a' + (i % 26);
    }

    for (i = 0; i < blocks; i++) {
        int n = write(fd, buf, BLOCK_SIZE);
        if (n != BLOCK_SIZE) {
            printf("Error: failed to write block %d\n", i);
            exit();
        }
    }

    close(fd);

    exit();
}
