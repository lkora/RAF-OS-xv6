#include "kernel/types.h"
#include "user.h"

void usage() {
    printf("Usage: setecho [0|1]\n"
            "  0: disable echo\n"
            "  1: enable echo\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage();
        exit();
    }

    int do_echo = atoi(argv[1]);
    if (do_echo != 0 && do_echo != 1) {
        usage();
        exit();
    }

    setecho(do_echo);

    if (do_echo) {
        printf("Echo enabled\n");
    } else {
        printf("Echo disabled\n");
    }

    exit();
}