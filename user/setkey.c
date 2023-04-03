#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"

char buf[512];

void setkey_usage(void)
{
    printf("Usage: setkey [-h|--help] [-s|--secret NEWKEY]\n"
            "Set the key for Caesar cipher.\n"
            "Options:\n"
            "  -h, --help\tShow this help message and exit\n"
            "  -s, --secret NEWKEY\tSet the key to NEWKEY\n");
}

int main(int argc, char *argv[])
{
    int i;
    int newkey = -1;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            setkey_usage();
            exit();
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--secret") == 0) {
            if (i + 1 >= argc) {
                printf("setkey: missing argument for --secret\n");
                setkey_usage();
                exit();
            }
            newkey = atoi(argv[++i]);
        }
        else {
            printf("setkey: unknown option %s\n", argv[i]);
            setkey_usage();
            exit();
        }
    }

    if (newkey == -1) {
        printf("Enter new key: ");
        gets(buf, sizeof(buf));
        newkey = atoi(buf);
    }

    if (setkey(newkey) < 0) {
        printf("setkey: failed to set key\n");
        exit();
    }

    exit();
}
