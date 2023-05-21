#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user.h"

#define MAX_WORD_SIZE 100

struct shared_data {
    char *filepath;
    int sentence_count;
    char longest_word[MAX_WORD_SIZE];
    int longest_word_size;
    char shortest_word[MAX_WORD_SIZE];
    int shortest_word_size;
    char text_longest_word[MAX_WORD_SIZE];
    int text_longest_word_size;
    int command;
    char text_shortest_word[MAX_WORD_SIZE];
    int text_shortest_word_size;
};

int main(int argc, char *argv[]) {
    // Initialize the shared data
    struct shared_data data = {0};
    if (argc > 1) {
        data.filepath = argv[1];
    } else {
        data.filepath = "../home/README";
    }
    data.sentence_count = 0;
    data.longest_word_size = 0;
    data.shortest_word_size = MAX_WORD_SIZE;
    data.text_longest_word_size = 0;
    data.command = 0;
    data.text_shortest_word_size = MAX_WORD_SIZE;

    // Share the data with child processes
    if (share_data("data", &data, sizeof(data)) < 0) {
        fprintf(2, "share_data failed\n");
        exit();
    }

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