#pragma once

#include <stddef.h>

struct struct_network {
    double **values;

    size_t len;
    size_t *layers;

    double **biases;
    double **costs;
    double ***weights; 
    // d1=[LAYER - 1]
    // d2=[Index of neuron (layer)]
    // d3=[Index of neuron (layer - 1)- 1]
};

typedef struct struct_network network;

network *init_network(size_t *layers, size_t len);
network *rand_init_network(size_t *layers, size_t len,
        double wmin, double wmax, double bmin, double bmax);

network *import_network(char *path);
void export_network(network *n, char *path);

network *xavier_init_network(size_t *layers, size_t len);
void free_network(network *network);

void print_network(network *n);
void network_to_graph(network *n, char *path);
network *he_init_network(size_t *layers, size_t len);
