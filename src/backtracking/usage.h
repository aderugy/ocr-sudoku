#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include "solver.h"
#include "filestream.h"
#include "allocator.h"


void usage()
{
    printf("Usage:\n");
    printf("./solver filename dim");
    errx(1, "give a right number of arguments");
}
