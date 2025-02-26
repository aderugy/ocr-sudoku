#include <pthread.h>
#include <emmintrin.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "thread_pool.h"
#include "bits/pthreadtypes.h"
#include "ocr.h"
#include "network.h"


ThreadPool* create_thread_pool(size_t num_threads)
{

    ThreadPool* pool = (ThreadPool*)malloc(num_threads * sizeof(ThreadPool));
    pool->threads = (pthread_t*)malloc(num_threads* sizeof(pthread_t));
    pool->num_threads = num_threads;
    pool->tasks_completed = 0;
    pool->total_tasks = 0;
    pool->task_state = NULL;

    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    return pool;
}

void destryoy_pool(ThreadPool* pool)
{

    free(pool->threads);
    free(pool->task_state);
    free(pool);
}

void* execute_task(void* arg)
{
    ThreadPool *pool = (ThreadPool*)arg;
    while(1)
    {
        pthread_mutex_lock(&pool->mutex);
        while(pool->tasks_completed >= pool-> total_tasks)
        {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        size_t task_index = pool->tasks_completed++;
        pthread_mutex_unlock(&pool->mutex);
        if(task_index < pool -> total_tasks)
        {
            if(pool->task_state[task_index] == 1)
            {
                //feed forward
            }
            else{}
        }
        else {
        break;
        }
    }
    pthread_exit(NULL);
}



void submit_task(ThreadPool* pool, size_t num_tasks, int* task_state)
{
    pool -> tasks_completed = 0;
    pool->total_tasks=num_tasks;
    pool->task_state = task_state;
    pthread_cond_broadcast(&pool->cond);

}



