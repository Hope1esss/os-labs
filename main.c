#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

pthread_mutex_t mutex;
const char *text;
const char *pattern;
int text_len;
int num_threads;
int pattern_len;

void *naive_multithread_search(void *arg)
{
    int thread_id = *(int *)arg;
    int segment_size = text_len / num_threads;
    int start_pos = thread_id * segment_size;
    int end_pos = (thread_id == num_threads - 1) ? text_len : start_pos + segment_size + pattern_len - 1;

    for (int i = start_pos; i <= end_pos - pattern_len; i++)
    {
        int j;
        for (j = 0; j < pattern_len; j++)
        {
            if (text[i + j] != pattern[j])
            {
                break;
            }
        }

        if (j == pattern_len)
        {
            pthread_mutex_lock(&mutex);
            printf("Thread %d found a match at position %d\n", thread_id, i);
            pthread_mutex_unlock(&mutex);
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s <text> <pattern> <max_threads>\n", argv[0]);
        return 1;
    }

    text = argv[1];
    pattern = argv[2];
    int max_threads = atoi(argv[3]);

    text_len = strlen(text);
    pattern_len = strlen(pattern);

    if (text_len < pattern_len)
    {
        num_threads = 1;
    }
    else
    {
        num_threads = (text_len / pattern_len) < max_threads ? (text_len / pattern_len) : max_threads;
    }

    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    pthread_mutex_init(&mutex, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, naive_multithread_search, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    pthread_mutex_destroy(&mutex);

    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Elapsed time: %.6f seconds\n", elapsed_time);
    printf("Search completed. Threads used: %d\n", num_threads);

    return 0;
}
