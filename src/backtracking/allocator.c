#include <stdio.h>
#include <stdlib.h>
#include <err.h>

unsigned int **allocGrid(unsigned int dimension)
{
    unsigned int **grid = NULL;
    grid = calloc(dimension, sizeof(unsigned int *));
    //the calloc function will create a new dynamic tab in memory 

    if (grid == NULL)
    {
        errx(EXIT_FAILURE, "Failing while allocating grid");
    }
    for (size_t i = 0; i < dimension; i++)
    {
        grid[i] = calloc(dimension, sizeof(unsigned int));
        if (grid[i] == NULL)
        {
            errx(EXIT_FAILURE, "Failing while allocating grid");
        }
    }
    return grid;
}

void freeGrid(int **grid, int dim)
{
    //this function will be used everywhere to free an array
    for(int i = 0; i<dim; ++i)
        free(grid[i]);

    free(grid);
}
