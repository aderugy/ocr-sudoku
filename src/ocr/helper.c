#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

//use these functions to generate random bits, for XOR tests
double drand(double min, double max) {
    return ((float) rand() / (float) RAND_MAX) * (max - min) + min;
}

double he_scale(size_t input_size) {
  return sqrt(2.0 / input_size);
}
double xavier_scale(size_t input_size, size_t output_size) {
    return sqrt(2.0 / (input_size + output_size));
}
double *get_random_bits(size_t n) {
    double *r = malloc(n * sizeof(double));

    for (size_t i = 0; i < n; i++) {
        r[i] = drand(0, 1);
        if (r[i] < 0.5)
            r[i] = 0;
        else
            r[i] = 1;
    }

    return r;
}

int is_decimal(const char *str) {
    if (NULL == str || 0 == str[0])
        return 0;

    int found = 0;
    for (size_t i = 0; str[i]; i++) {
        if ('.' == str[i] || ',' == str[i]) {
            if (found)
                return 0;
            found = 1;
        }
        else if (str[i] < '0' || str[i] > '9')
            return 0;
    }

    return 1;
}

int is_int(const char *str) {
    if (NULL == str || 0 == str[0])
        return 0;

    for (size_t i = 0; str[i]; i++)
        if (str[i] < '0' || str[i] > '9')
            return 0;
    return 1;
}

double diff_timespec(const struct timespec* t1, const struct timespec* t0) {
    double second = difftime(t1->tv_sec, t0->tv_sec);
    return second + ((double)t1->tv_nsec - (double)t0->tv_nsec) / 1e9;
}
