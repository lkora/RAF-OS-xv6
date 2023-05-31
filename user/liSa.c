#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user.h"
#include "shared_data.h"


void find_words(char *line, struct shared_data *data) {
    // Find the shortest and longest word in the current sentence
    int word_start = -1;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == ' ' || line[i] == '.' || line[i] == ',' || line[i] == ';' || line[i] == ':' || line[i] == '!' || line[i] == '?') {
            if (word_start != -1) {
                int word_size = i - word_start;
                if (word_size > data->longest_word_size) {
                    data->longest_word_size = word_size;
                    memmove(data->longest_word, &line[word_start], word_size);
                    data->longest_word[word_size] = '\0';
                }
                if (word_size < data->shortest_word_size) {
                    data->shortest_word_size = word_size;
                    memmove(data->shortest_word, &line[word_start], word_size);
                    data->shortest_word[word_size] = '\0';
                }
                word_start = -1;
            }
        } else if (word_start == -1) {
            word_start = i;
        }
    }

    // Update the global extrema
    if (data->longest_word_size > data->text_longest_word_size) {
        data->text_longest_word_size = data->longest_word_size;
        strcpy(data->text_longest_word, data->longest_word);
    }
    if (data->shortest_word_size < data->text_shortest_word_size) {
        data->text_shortest_word_size = data->shortest_word_size;
        strcpy(data->text_shortest_word, data->shortest_word);
    }
}

int main(void) {
    // Access the shared data
    struct shared_data *data;
    if (get_data("data", (void **)&data) < 0) {
        fprintf(2, "get_data failed\n");
        exit();
    }

    fprintf(2, "LISA - Addr shared: %p\n", data);
    fprintf(2, "%c %d %d %d %d\n", *data->filepath, data->sentence_count, data->longest_word_size, data->shortest_word_size, data->text_shortest_word_size);
    printf("Child: filepath = %s, data->filepath = %p\n", data->filepath, data->filepath);

    // Open the file
    int fd = open(data->filepath, O_RDONLY);
    if (fd < 0) {
        fprintf(2, "open failed\n");
        exit();
    }

    // Process the file
    char line[MAX_LINE_SIZE];
    while (1) {
        // Check for commands from coMMa
        while (data->command == 1) {
            sleep(1);
        }
        if (data->command == 3) {
            break;
        }

        // Read a line from the file
        int n = 0;
        while (1) {
            char c;
            int r = read(fd, &c, 1);
            if (r < 0) {
                fprintf(2, "read failed\n");
                close(fd);
                exit();
            }
            if (r == 0 || c == '\n') {
                break;
            }
            line[n++] = c;
        }
        if (n == 0) {
            break;
        }
        line[n] = '\0';

        // Find the shortest and longest word in the current sentence
        find_words(line, data);

        // Increment the sentence count
        data->sentence_count++;

        // Simulate data analysis
        sleep(150);
    }

    // Close the file
    close(fd);

    fprintf(1, "Task finished\n");

    exit();
}
