#include "wordcount.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

hm_object hash_map[MAX_HASH_SIZE];
int hash_map_size = 0;
char stop_words[100][MAX_WORD_LEN];
int stop_words_count = 0;
pthread_mutex_t hash_map_locks[MAX_HASH_SIZE];

void map_init()
{
    for (int i = 0; i < MAX_HASH_SIZE; i++)
    {
        hash_map[i].key = NULL;
        hash_map[i].value = 0;
        pthread_mutex_init(&hash_map_locks[i], NULL);
    }
}

void add_stopword(char *word)
{
    strcpy(stop_words[stop_words_count++], word);
}

void map_clear() {
    for (int i = 0; i < MAX_HASH_SIZE; i++) {
        pthread_mutex_lock(&hash_map_locks[i]);
        if (hash_map[i].key != NULL) {
            free(hash_map[i].key);
            hash_map[i].key = NULL;
            hash_map[i].value = 0;
        }
        pthread_mutex_unlock(&hash_map_locks[i]);
    }
    hash_map_size = 0;
}

int is_stop_word(char *word)
{
    for (int i = 0; i < stop_words_count; i++)
    {
        if (strcmp(stop_words[i], word) == 0)
        {
            return 1;
        }
    }
    return 0;
}

unsigned long hash(char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash % MAX_HASH_SIZE;
}

void map_add_word_count(char *word, int value, char *file_name)
{
    if (is_stop_word(word))
    {
#ifdef DEBUG
        printf("Word %s is a stop word. Not adding to hash map.\n", word);
#endif
        return;
    }

    unsigned long index = hash(word);
#ifdef DEBUG
    printf("Calculated hash value for word %s: %lu\n", word, index);
#endif
    pthread_mutex_lock(&hash_map_locks[index]);
    if (hash_map[index].key == NULL)
    {
#ifdef DEBUG
        printf("Adding new entry for word %s at index %lu\n", word, index);
#endif
        hash_map[index].key = malloc(MAX_WORD_LEN);
        hash_map[index].file_name = malloc(MAX_WORD_LEN);
        
        strcpy(hash_map[index].key, word);
        strcpy(hash_map[index].file_name, file_name);
        hash_map[index].value = value;
        hash_map_size++;
    }
    else if (strcmp(hash_map[index].key, word) == 0 && strcmp(hash_map[index].file_name, file_name) == 0)
    {
#ifdef DEBUG
        printf("Incrementing count for word %s at index %lu\n", word, index);
#endif
        hash_map[index].value += value;
    }
    else
    {
        // handle collision
#ifdef DEBUG
        printf("Collision detected for word %s at index %lu\n", word, index);
#endif
        int found = 0;
        for (unsigned long i = index + 1; i != index; i = (i + 1) % MAX_HASH_SIZE)
        {
            pthread_mutex_lock(&hash_map_locks[i]);
            if (hash_map[i].key == NULL)
            {
#ifdef DEBUG
                printf("Adding new entry for word %s at index %lu\n", word, i);
#endif
                hash_map[i].key = malloc(MAX_WORD_LEN);
                hash_map[i].file_name = malloc(MAX_WORD_LEN);
                strcpy(hash_map[i].key, word);
                strcpy(hash_map[i].file_name, file_name);
                hash_map[i].value = value;
                hash_map_size++;
                found = 1;
                pthread_mutex_unlock(&hash_map_locks[i]);
                break;
            }
            else if (strcmp(hash_map[i].key, word) == 0 && strcmp(hash_map[i].file_name, file_name) == 0)
            {
#ifdef DEBUG
                printf("Incrementing count for word %s at index %lu\n", word, i);
#endif
                hash_map[i].value += value;
                found = 1;
                pthread_mutex_unlock(&hash_map_locks[i]);
                break;
            }
            pthread_mutex_unlock(&hash_map_locks[i]);
        }
        if (!found)
        {
            printf("Hash map is full!\n");
        }
    }
    pthread_mutex_unlock(&hash_map_locks[index]);
}

search_result *map_get_frequency(char *word, char *file_name)
{
    search_result *result = malloc(sizeof(search_result));
    result->key = malloc(MAX_WORD_LEN);
    strcpy(result->key, word);
    result->value = -1;

    unsigned long index = hash(word);
#ifdef DEBUG
    printf("Calculated hash value for word %s: %lu\n", word, index);
#endif
    pthread_mutex_lock(&hash_map_locks[index]);
    if (hash_map[index].key != NULL && strcmp(hash_map[index].key, word) == 0 && strcmp(hash_map[index].file_name, file_name) == 0)
    {
        result->value = hash_map[index].value;
        pthread_mutex_unlock(&hash_map_locks[index]);
        return result;
    }
    pthread_mutex_unlock(&hash_map_locks[index]);

    // handle collision
#ifdef DEBUG
    printf("Checking for collision for word %s at index %lu\n", word, index);
#endif
    for (unsigned long i = index + 1; i != index; i = (i + 1) % MAX_HASH_SIZE)
    {
        pthread_mutex_lock(&hash_map_locks[i]);
        if (hash_map[i].key != NULL && strcmp(hash_map[i].key, word) == 0 && strcmp(hash_map[i].file_name, file_name) == 0)
        {
            result->value = hash_map[i].value;
            pthread_mutex_unlock(&hash_map_locks[i]);
            return result;
        }
        pthread_mutex_unlock(&hash_map_locks[i]);
    }

    return result;
}

int map_get_total_frequency(char *word)
{
    int total = 0;
    for (int i = 0; i < MAX_HASH_SIZE; i++)
    {
        pthread_mutex_lock(&hash_map_locks[i]);
        if (hash_map[i].key != NULL && strcmp(hash_map[i].key, word) == 0)
        {
            total += hash_map[i].value;
        }
        pthread_mutex_unlock(&hash_map_locks[i]);
    }
    return total;
}
