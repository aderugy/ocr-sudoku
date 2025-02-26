#include <math.h>

#include <emmintrin.h>
#include <pthread.h>

#include "thread_pool.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "err.h"
#include "helper.h"
#include "network.h"
#include "ocr.h"

#define NUM_THREADS 8
pthread_mutex_t mutex;

double softmax_denominator(double x, size_t total_classes) {
  double sum = 0.0;
  for (size_t i = 0; i < total_classes; ++i) {
    sum += exp(x);
  }
  return sum;
}

void softmax(unsigned int k, double *z) {
  double max_val = z[0];
  for (unsigned i = 1; i < k; i++) {
    if (z[i] > max_val) {
      max_val = z[i];
    }
  }

  double sum = 0;
  for (unsigned i = 0; i < k; i++) {
    sum += exp(z[i] - max_val);
  }

  sum = sum < 1e-8 ? 1e-8 : sum;
  for (unsigned i = 0; i < k; i++) {
    z[i] = exp(z[i] - max_val) / sum;
  }
}

size_t read_output(network *n) {
  size_t last = n->len - 1;
  size_t max = 0;
  for (size_t i = 1; i < n->layers[last]; i++) {
    if (n->values[last][i] > n->values[last][max])
      max = i;
  }
  return max;
}

double softmax_derivative(double softmax_value) {
  return softmax_value * (1.0 - softmax_value);
}

double sigmoid(double x) { return 1 / (1 + exp(-x)); }

double prime_sigmoid(double x) { return x * (1 - x); }

void feed_forward(network *n, double *inputs) {
  for (size_t i = 0; i < n->layers[0]; i++)
    n->values[0][i] = inputs[i];

  for (size_t layer = 1; layer < n->len - 1; layer++) {
    double **wmat = n->weights[layer - 1];
    double *v_inp = n->values[layer - 1];
    double *v_out = n->values[layer];
    double *b_row = n->biases[layer - 1];

    for (size_t i = 0; i < n->layers[layer]; i++) {
      double *wrow = wmat[i];
      double sum = 0;
      __m128d sum_vector = _mm_setzero_pd();

      for (size_t j = 0; j < n->layers[layer - 1]; j += 2) {
        __m128d v_inp_vector = _mm_loadu_pd(&v_inp[j]);
        __m128d wrow_vector = _mm_loadu_pd(&wrow[j]);
        sum_vector =
            _mm_add_pd(sum_vector, _mm_mul_pd(v_inp_vector, wrow_vector));
      }
      sum += sum_vector[0] + sum_vector[1];

      for (size_t j = n->layers[layer - 1] & ~1; j < n->layers[layer - 1];
           j++) {
        sum += v_inp[j] * wrow[j];
      }

      v_out[i] = sigmoid(sum + b_row[i]);
    }
  }

  size_t last = n->len - 1;
  double *output_layer_input = n->values[last - 1];
  double *output_layer_output = n->values[last];
  double *output_layer_biases = n->biases[last - 1];

  for (size_t i = 0; i < n->layers[last]; i++) {
    double sum = output_layer_biases[i];

    for (size_t j = 0; j < n->layers[last - 1]; j++) {
      sum += n->weights[last - 1][i][j] * output_layer_input[j];
    }

    output_layer_output[i] = sum;
  }

  softmax(n->layers[last], output_layer_output);
}

/*
void feed_forward(network *n, double *inputs) {
  for (size_t i = 0; i < n->layers[0]; i++)
    n->values[0][i] = inputs[i];
  for (size_t layer = 1; layer < n->len - 1; layer++) {
    double **wmat = n->weights[layer - 1];
    double *v_inp = n->values[layer - 1];
    double *v_out = n->values[layer];
    double *b_row = n->biases[layer - 1];

    for (size_t i = 0; i < n->layers[layer]; i++) {
      double *wrow = wmat[i];
      double sum = 0;

      for (size_t j = 0; j < n->layers[layer - 1]; j++) {
        sum += v_inp[j] * wrow[j];
      }

      v_out[i] = sigmoid(sum + b_row[i]);
    }
  }
  size_t last = n->len - 1;
  double *output_layer_input = n->values[last - 1];
  double *output_layer_output = n->values[last];
  double *output_layer_biases = n->biases[last - 1];

  for (size_t i = 0; i < n->layers[last]; i++) {
    double sum = output_layer_biases[i];

    for (size_t j = 0; j < n->layers[last - 1]; j++) {
      sum += n->weights[last - 1][i][j] * output_layer_input[j];
    }

    output_layer_output[i] = sum;
  }

  softmax(n->layers[last], output_layer_output);
}


void* feed_forward_task(void *arg) {
    FeedForwardTask *task = (FeedForwardTask*)arg;
    network *n = task->n;
    double *inputs = task->inputs;
    double *output = task->output;
    size_t layer = task->layer;

    double **wmat = n->weights[layer - 1];
    double *v_inp = n->values[layer - 1];
    double *v_out = n->values[layer];
    double *b_row = n->biases[layer - 1];

    for (size_t i = 0; i < n->layers[layer]; i++) {
        double *wrow = wmat[i];
        double sum = 0;
        __m128d sum_vector = _mm_setzero_pd();

        for (size_t j = 0; j < n->layers[layer - 1]; j += 2) {
            __m128d v_inp_vector = _mm_loadu_pd(&v_inp[j]);
            __m128d wrow_vector = _mm_loadu_pd(&wrow[j]);
            sum_vector = _mm_add_pd(sum_vector, _mm_mul_pd(v_inp_vector,
wrow_vector));
        }
        sum += sum_vector[0] + sum_vector[1];

        for (size_t j = n->layers[layer - 1] & ~1; j < n->layers[layer - 1];
j++) { sum += v_inp[j] * wrow[j];
        }

        v_out[i] = sigmoid(sum + b_row[i]);
    }

    if (layer == n->len - 1) {
        double *output_layer_input = n->values[layer - 1];

        for (size_t i = 0; i < n->layers[layer]; i++) {
            double sum = n->biases[layer - 1][i];

            for (size_t j = 0; j < n->layers[layer - 1]; j++) {
                sum += n->weights[layer - 1][i][j] * output_layer_input[j];
            }

            output[i] = sum;
        }

        softmax(n->layers[layer], output);
    }

    pthread_exit(NULL);
}

void feed_forward(network *n, double *inputs, double *output, ThreadPool *pool)
{ for (size_t i = 0; i < n->layers[0]; i++) { n->values[0][i] = inputs[i];
    }

    size_t num_layers = n->len - 1;
    size_t num_tasks = num_layers;
    int *task_states = (int*)malloc(num_tasks * sizeof(int));
    FeedForwardTask *tasks = (FeedForwardTask*)malloc(num_tasks *
sizeof(FeedForwardTask)); for (size_t i = 0; i < num_tasks; i++) {
        task_states[i] = 1;
        tasks[i].n = n;
        tasks[i].inputs = inputs;
        tasks[i].output = output;
        tasks[i].layer = i + 1;
    }

    submit_task(pool, num_tasks, task_states);
    while (pool->tasks_completed < num_tasks) {
        // Optionally, perform other tasks in the main thread
    }
    free(task_states);
    free(tasks);
}
*/

void *back_prop_thread(void *arg) {
  struct ThreadData *data = (struct ThreadData *)arg;
  network *n = data->n;
  size_t start = data->start;
  size_t end = data->end;
  pthread_mutex_t *mutex = data->mutex;

  for (size_t i = start; i < end; i++) {
    size_t last = n->len - 1;
    double v = n->values[last][i];
    n->costs[last - 1][i] = v * (1 - v) * (v - data->expected[i]);
  }

  pthread_mutex_unlock(mutex);

  pthread_exit(NULL);
}

void back_prop(network *n, int *expected) {
  size_t last = n->len - 1;
  pthread_t threads[NUM_THREADS];
  struct ThreadData threadData[NUM_THREADS];
  pthread_mutex_init(&mutex, NULL);

  size_t neurons_per_thread = n->layers[last] / NUM_THREADS;
  size_t remaining_neurons = n->layers[last] % NUM_THREADS;
  size_t current_start = 0;

  for (size_t i = 0; i < NUM_THREADS; i++) {
    threadData[i].n = n;
    threadData[i].start = current_start;
    threadData[i].end =
        current_start + neurons_per_thread + (i < remaining_neurons ? 1 : 0);
    threadData[i].mutex = &mutex;
    threadData[i].expected = expected;
    current_start = threadData[i].end;
    pthread_create(&threads[i], NULL, back_prop_thread, &threadData[i]);
  }

  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);

  for (size_t i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&mutex);
}

void *learn_thread(void *arg) {
  struct ThreadData *data = (struct ThreadData *)arg;
  network *n = data->n;
  size_t start = data->start;
  size_t end = data->end;
  pthread_mutex_t *mutex = data->mutex;
  double speed = data->speed;

  for (size_t i = start; i < end; i++) {
    for (size_t j = 0; j < n->layers[i]; j++) {
      for (size_t k = 0; k < n->layers[i + 1]; k++) {
        n->weights[i][k][j] -= speed * n->values[i][j] * n->costs[i][k];
      }
    }

    for (size_t j = 0; j < n->layers[i + 1]; j++) {
      n->biases[i][j] -= speed * n->costs[i][j];
    }
  }

  pthread_mutex_unlock(mutex);

  pthread_exit(NULL);
}

void learn(network *n, double speed) {
  pthread_t threads[NUM_THREADS];
  struct ThreadData threadData[NUM_THREADS];
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);

  size_t layers_per_thread = (n->len - 1) / NUM_THREADS;
  size_t remaining_layers = (n->len - 1) % NUM_THREADS;
  size_t current_start = 0;

  for (size_t i = 0; i < NUM_THREADS; i++) {
    threadData[i].n = n;
    threadData[i].start = current_start;
    threadData[i].end =
        current_start + layers_per_thread + (i < remaining_layers ? 1 : 0);
    threadData[i].mutex = &mutex;
    threadData[i].speed = speed;
    current_start = threadData[i].end;
    pthread_create(&threads[i], NULL, learn_thread, &threadData[i]);
  }

  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);

  for (size_t i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&mutex);
}
