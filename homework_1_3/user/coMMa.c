#include "kernel/types.h"
#include "user.h"
#include "shared_data.h"


int main(void) {
    // Access the shared data
    struct shared_data *data;
    if (get_data("data", (void **)&data) < 0) {
        fprintf(2, "get_data failed\n");
        exit();
    }

    // Accept commands from the user in a loop
    while (1) {
        fprintf(1, "> ");
        char cmd[20];
        gets(cmd, sizeof(cmd));
        cmd[strlen(cmd) - 1] = '\0';

        if (strcmp(cmd, "latest") == 0) {
            // Print the latest sentence data
            fprintf(1, "Sentence: %d\n", data->sentence_count);
            fprintf(1, "Longest word: %s (%d)\n", data->longest_word, data->longest_word_size);
            fprintf(1, "Shortest word: %s (%d)\n", data->shortest_word, data->shortest_word_size);
        } else if (strcmp(cmd, "global extrema") == 0) {
            // Print the global extrema
            fprintf(1, "Longest word: %s (%d)\n", data->text_longest_word, data->text_longest_word_size);
            fprintf(1, "Shortest word: %s (%d)\n", data->text_shortest_word, data->text_shortest_word_size);
        } else if (strcmp(cmd, "pause") == 0) {
            // Pause text processing
            data->command = 1;
        } else if (strcmp(cmd, "resume") == 0) {
            // Resume text processing
            data->command = 2;
        } else if (strcmp(cmd, "end") == 0) {
            // End text processing
            data->command = 3;
            break;
        }
    }

    exit();
}
