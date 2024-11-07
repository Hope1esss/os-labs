#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_MATCHES 100000  // Максимальное количество совпадений, которое мы можем хранить

// Структура для передачи данных в поток
typedef struct {
    const char* text;
    const char* pattern;
    int start;
    int end;
    int* results;
    int* result_count;
    pthread_mutex_t* mutex;
} ThreadData;

// Функция, выполняющая наивный поиск подстроки в заданном диапазоне
void* naive_search(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    const char* text = data->text;
    const char* pattern = data->pattern;
    int pattern_len = strlen(pattern);

    for (int i = data->start; i <= data->end - pattern_len + 1; i++) {
        if (strncmp(&text[i], pattern, pattern_len) == 0) {
            pthread_mutex_lock(data->mutex); // Блокируем доступ к результатам
            if (*data->result_count < MAX_MATCHES) {
                data->results[(*data->result_count)++] = i;
            }
            pthread_mutex_unlock(data->mutex); // Разблокируем
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <text> <pattern> <max_threads>\n", argv[0]);
        return 1;
    }

    const char* text = argv[1];
    const char* pattern = argv[2];
    int max_threads = atoi(argv[3]);

    if (max_threads <= 0) {
        fprintf(stderr, "Error: max_threads should be a positive integer\n");
        return 1;
    }

    int text_len = strlen(text);
    int pattern_len = strlen(pattern);

    // Определяем количество потоков
    int num_threads = (text_len < pattern_len) ? 1 : (text_len / pattern_len < max_threads ? text_len / pattern_len : max_threads);

    int chunk_size = text_len / num_threads;
    int results[MAX_MATCHES];
    int result_count = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[num_threads];

    // Создаем потоки
    for (int i = 0; i < num_threads; i++) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? text_len - 1 : (i + 1) * chunk_size + pattern_len - 2;

        // Заполняем структуру для передачи в поток
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->text = text;
        data->pattern = pattern;
        data->start = start;
        data->end = end;
        data->results = results;
        data->result_count = &result_count;
        data->mutex = &mutex;

        // Запускаем поток
        pthread_create(&threads[i], NULL, naive_search, (void*)data);
    }

    // Ожидаем завершения всех потоков
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    // Выводим результаты
    printf("Found matches at positions: ");
    for (int i = 0; i < result_count; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");

    return 0;
}
