#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user.h"
#include "shared_data.h"

int main(int argc, char *argv[]) {
    // Initialize the shared data
    
    char* filepath = argc > 1 ? argv[1] : "/home/README\0";
    
    struct shared_data data = {
        .filepath = filepath,
        .sentence_count = 0,
        .longest_word_size = 0,
        .shortest_word_size = MAX_WORD_SIZE,
        .text_longest_word_size = 0,
        .command = 0,
        .text_shortest_word_size = MAX_WORD_SIZE,
    };

    fprintf(2, "data struct size: %d\n", sizeof(data));
    // Share the data with child processes
    if (share_data("data", (void*)&data, sizeof(data)) < 0) {
        fprintf(2, "share_data failed\n");
        exit();
    }

    fprintf(2, "DALLE - Addr shared: %p\n", data);
    fprintf(2, "%s %d %d %d %d\n", data.filepath, data.sentence_count, data.longest_word_size, data.shortest_word_size, data.text_shortest_word_size);
    printf("Parent: filepath = %s, data.filepath = %p\n", data.filepath, data.filepath);


    // Start the coMMa child process
    int pid1 = fork();
    if (pid1 < 0) {
        fprintf(2, "fork failed\n");
        exit();
    }
    if (pid1 == 0) {
        // Child process
        char *args[] = {"coMMa", 0};
        exec("/bin/coMMa", args);
        fprintf(2, "exec failed\n");
        exit();
    }

    // Start the liSa child process
    int pid2 = fork();
    if (pid2 < 0) {
        fprintf(2, "fork failed\n");
        exit();
    }
    if (pid2 == 0) {
        // Child process
        char *args[] = {"liSa", 0};
        exec("/bin/liSa", args);
        fprintf(2, "exec failed\n");
        exit();
    }

    // Wait for both child processes to exit
    wait();
    wait();

    exit();
}