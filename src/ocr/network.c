#include <errno.h>
#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "network.h"

// init the eural_network, defined by a struct containing
// a vector for values
network *init_network(size_t *layers, size_t len) {
  network *n = malloc(sizeof(network));

  double **values = malloc(len * sizeof(double *));

  for (size_t i = 0; i < len; i++) {
    values[i] = calloc(layers[i], sizeof(double));
  }

  double **biases = malloc((len - 1) * sizeof(double *));
  double **costs = malloc((len - 1) * sizeof(double *));
  double ***weights = malloc((len - 1) * sizeof(double *));

  for (size_t i = 0; i < len - 1; i++) {
    biases[i] = malloc(layers[i + 1] * sizeof(double));
    double *b_row = biases[i];

    costs[i] = malloc(layers[i + 1] * sizeof(double));
    double *c_row = costs[i];

    weights[i] = malloc(layers[i + 1] * sizeof(double));
    double **w_mat = weights[i];

    for (size_t j = 0; j < layers[i + 1]; j++) {
      c_row[j] = 0;
      b_row[j] = 0;
      w_mat[j] = malloc(layers[i] * sizeof(double));
      double *w_row = w_mat[j];

      for (size_t k = 0; k < layers[i]; k++) {
        w_row[k] = 0;
      }
    }
  }

  n->values = values;
  n->len = len;
  n->layers = layers;

  n->costs = costs;
  n->biases = biases;
  n->weights = weights;

  return n;
}

network *he_init_network(size_t *layers, size_t len) {
  network *n = init_network(layers, len);

  for (size_t i = 0; i < n->len - 1; i++) {
    for (size_t j = 0; j < n->layers[i + 1]; j++) {
      n->biases[i][j] = drand(-1.0, 1.0) * he_scale(n->layers[i]);
      for (size_t k = 0; k < n->layers[i]; k++) {
        n->weights[i][j][k] = drand(-1.0, 1.0) * he_scale(n->layers[i]);
      }
    }
  }
  return n;
}
network *xavier_init_network(size_t *layers, size_t len) {
  network *n = init_network(layers, len);

  for (size_t i = 0; i < n->len - 1; i++) {
    for (size_t j = 0; j < n->layers[i + 1]; j++) {
      n->biases[i][j] =
          drand(-1.0, 1.0) * xavier_scale(n->layers[i], n->layers[i + 1]);

      for (size_t k = 0; k < n->layers[i]; k++) {
        n->weights[i][j][k] =
            drand(-1.0, 1.0) * xavier_scale(n->layers[i], n->layers[i + 1]);
      }
    }
  }
  return n;
}

network *rand_init_network(size_t *layers, size_t len, double wmin, double wmax,
                           double bmin, double bmax) {
  network *n = init_network(layers, len);

  for (size_t i = 0; i < n->len - 1; i++) {
    for (size_t j = 0; j < n->layers[i + 1]; j++) {
      n->biases[i][j] = drand(bmin, bmax);
      for (size_t k = 0; k < n->layers[i]; k++) {
        n->weights[i][j][k] = drand(wmin, wmax);
      }
    }
  }

  return n;
}

network *import_network(char *path) {
  FILE *in;
  in = fopen(path, "rb");

  if (NULL == in)
      errx(errno, "%s", "Couldn't open file.");

  network *n = malloc(sizeof(network));

  fread(&(n->len), sizeof(size_t), 1, in);
  n->layers = malloc(n->len * sizeof(size_t));

  n->values = malloc(n->len * sizeof(double *));
  n->costs = malloc((n->len - 1) * sizeof(double *));
  n->biases = malloc((n->len - 1) * sizeof(double *));
  n->weights = malloc((n->len - 1) * sizeof(double *));

  for (size_t i = 0; i < n->len; i++) {
    fread(&(n->layers[i]), sizeof(size_t), 1, in);
    n->values[i] = malloc(n->layers[i] * sizeof(double));
  }

  for (size_t i = 0; i < n->len - 1; i++) {
    n->costs[i] = malloc(n->layers[i + 1] * sizeof(double));
    n->biases[i] = malloc(n->layers[i + 1] * sizeof(double));
    n->weights[i] = malloc(n->layers[i + 1] * sizeof(double *));

    for (size_t j = 0; j < n->layers[i + 1]; j++) {
      fread(&(n->costs[i][j]), sizeof(double), 1, in);
      fread(&(n->biases[i][j]), sizeof(double), 1, in);
      n->weights[i][j] = malloc(n->layers[i] * sizeof(double));

      for (size_t k = 0; k < n->layers[i]; k++) {
        fread(&(n->weights[i][j][k]), sizeof(double), 1, in);
      }
    }
  }

  fclose(in);
  return n;
}

void export_network(network *n, char *path) {
  FILE *out;
  out = fopen(path, "wb");

  if (out == NULL) {
    errx(EXIT_FAILURE, "Could not open file %s.", path);
    return;
  }

  fwrite(&(n->len), sizeof(size_t), 1, out);
  for (size_t i = 0; i < n->len; i++)
    fwrite(&(n->layers[i]), sizeof(size_t), 1, out);

  for (size_t i = 0; i < n->len - 1; i++) {
    for (size_t j = 0; j < n->layers[i + 1]; j++) {
      fwrite(&(n->costs[i][j]), sizeof(double), 1, out);
      fwrite(&(n->biases[i][j]), sizeof(double), 1, out);

      for (size_t k = 0; k < n->layers[i]; k++) {
        fwrite(&(n->weights[i][j][k]), sizeof(double), 1, out);
      }
    }
  }

  fclose(out);
}

void free_network(network *n) {
  for (size_t i = 0; i < n->len; i++)
    free((n->values)[i]);
  free(n->values);

  for (size_t i = 0; i < n->len - 1; i++) {
    for (size_t j = 0; j < n->layers[i + 1]; j++)
      free(n->weights[i][j]);

    free(n->costs[i]);
    free(n->biases[i]);
    free(n->weights[i]);
  }
  free(n->costs);
  free(n->biases);
  free(n->weights);

  free(n->layers);
  free(n);
}
