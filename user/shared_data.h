#define MAX_WORD_SIZE 100
#define MAX_LINE_SIZE 500

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