#include "wordcount.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

scanned_file scanned_files[100];
int scanned_files_count = 0;
pthread_mutex_t scanned_files_lock;

void scanner_init()
{
    pthread_mutex_init(&scanned_files_lock, NULL);
}

time_t get_last_mod_time(char *file_name)
{
    struct stat attr;
    stat(file_name, &attr);
    return attr.st_mtime;
}

int has_been_scanned(char *file_name, time_t mod_time)
{
    for (int i = 0; i < scanned_files_count; i++)
    {
        if (strcmp(scanned_files[i].file_name, file_name) == 0 && difftime(scanned_files[i].mod_time, mod_time) == 0.0)
            return 1;
    }
    return 0;
}

void add_scanned_file(char *file_name, time_t mod_time)
{
    strcpy(scanned_files[scanned_files_count].file_name, file_name);
    scanned_files[scanned_files_count++].mod_time = mod_time;
}

void to_lower_case(char *word)
{
    for (size_t i = 0; i < strlen(word); i++)
    {
        if (word[i] >= 65 && word[i] <= 90)
            word[i] += 32;
    }
}

void *scanner_work(void *_args)
{
    char *file_name = (char *)_args;

#ifdef DEBUG
    printf("Scanner thread started for file %s\n", file_name);
#endif

    while (1)
    {
        FILE *fp = fopen(file_name, "r");
        if (fp == NULL)
        {
            printf("Error opening file %s\n", file_name);
            exit(1);
        }

        time_t mod_time = get_last_mod_time(file_name);

        pthread_mutex_lock(&scanned_files_lock);

        if (has_been_scanned(file_name, mod_time))
        {
            pthread_mutex_unlock(&scanned_files_lock);

            printf("File %s has already been scanned and has not been modified. Sleeping for 5 seconds.\n", file_name);
            sleep(5);
            continue;
        }

        printf("Scanning file %s\n", file_name);
        add_scanned_file(file_name, mod_time);

        pthread_mutex_unlock(&scanned_files_lock);

        char word[MAX_WORD_LEN];

        while (fscanf(fp, "%63s", word) == 1)
        {
            to_lower_case(word);

#ifdef DEBUG
            printf("Adding word %s to hash map\n", word);
#endif
            map_add_word_count(word, 1);
        }

        fclose(fp);

#ifdef DEBUG
        printf("Finished scanning file %s. Sleeping for 5 seconds.\n", file_name);
#endif

        sleep(5);
    }

    return NULL;
}
