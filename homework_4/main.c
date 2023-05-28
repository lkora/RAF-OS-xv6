#include "wordcount.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int thread_count = 0;
scanner_thread threads[MAX_THREAD_COUNT];

void start_scanner_thread(char *file_name)
{
    for (int i = 0; i < thread_count; i++)
    {
        if (strcmp(threads[i].file_name, file_name) == 0)
        {
            return;
        }
    }

    strcpy(threads[thread_count].file_name, file_name);

    // Create a copy of the file_name parameter to pass to the new thread
    char *file_name_copy = malloc(strlen(file_name) + 1);
    strcpy(file_name_copy, file_name);

    pthread_create(&threads[thread_count++].thread_id, NULL, &scanner_work, (void *)file_name_copy);
}

int main(int argc, char **argv)
{
    scanner_init();
    map_init();

    if (argc > 2 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--stop") == 0))
    {
        FILE *fp = fopen(argv[2], "r");
        if (fp == NULL)
        {
            printf("Error opening stop words file\n");
            exit(1);
        }

        char word[MAX_WORD_LEN];

        while (fscanf(fp, "%63s", word) == 1)
        {
            add_stopword(word);
        }

        fclose(fp);

        argc -= 2;
        argv += 2;
    }

    while (1)
    {
        char command[MAX_WORD_LEN];

        scanf("%63s", command);

        if (strcmp(command, "_count_") == 0)
        {
            char file_name[MAX_WORD_LEN];
            scanf("%63s", file_name);

            start_scanner_thread(file_name);
        }
        else if (strcmp(command, "_stop_") == 0)
        {
            exit(0);
        }
        else
        {
            int total = map_get_total_frequency(command);
            printf("%d\n", total);
        }
    }

    return 0;
}
