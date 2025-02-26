#include <pthread.h>

typedef struct
{
    pthread_t* threads;
    size_t num_threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    size_t tasks_completed;
    size_t total_tasks;
    int *task_state;
} ThreadPool;

ThreadPool* create_thread_pool(size_t num_threads);

void destryoy_pool(ThreadPool* pool);

void* execute_task(void* arg);

void submit_task(ThreadPool* pool, size_t num_tasks, int* task_state);
