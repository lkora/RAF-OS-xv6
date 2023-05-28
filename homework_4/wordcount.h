#ifndef WORDCOUNT_H
#define WORDCOUNT_H

#include <time.h>
#include <pthread.h>

#define MAX_WORD_LEN 64
#define LETTERS 26
#define MAX_HASH_SIZE 1000000
#define MAX_THREAD_COUNT 100

typedef struct hm_object {
    char* key;
    int value;
} hm_object;

typedef struct search_result {
    char* key;
    int value;
} search_result;

typedef struct scanned_file {
    char file_name[256];
    time_t mod_time;
} scanned_file;

typedef struct {
    pthread_t thread_id;
    char file_name[MAX_WORD_LEN];
} scanner_thread;


extern void scanner_init();
extern void *scanner_work(void *_args);
extern void map_init();
extern void map_add_word_count(char *word, int value);
extern search_result *map_get_frequency(char *word);
extern void add_stopword(char* word);
extern void map_clear();

#endif
