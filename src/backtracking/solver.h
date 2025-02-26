#pragma once

unsigned int ValidRow(unsigned int **grid, unsigned int x, unsigned int y,
        unsigned int val, unsigned int dim);

unsigned int ValidColumn(unsigned int **grid, unsigned int x, unsigned int y,
        unsigned int val, unsigned int dim);

unsigned int ValidSquare(unsigned int **grid, unsigned int x, unsigned int y,
        unsigned int val, unsigned int dim);

unsigned int BePlaced(unsigned int **grid, unsigned int x, unsigned int y,
        unsigned int val, unsigned int dim);

unsigned int solve(unsigned int **grid,unsigned int x, unsigned int y,
        unsigned int dim);

