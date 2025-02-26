#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hough.h"
// noir 0
// blanc 1

const int theta_s = 180;             // Adjust as needed for precision
const double rho = 1;                // Distance resolution in pixels
const double theta = M_PI / theta_s; // Angle resolution in radians

// HOUGH ALGO

Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
  if (x < 0 || y < 0 || x >= surface->w || y >= surface->h)
    return 0; // Out of bounds

  Uint32 *pixels = (Uint32 *)surface->pixels;
  Uint32 pixel_value = pixels[y * surface->w + x];

  return pixel_value;
}

int accu_maxvalue(int **accumulator, int rows, int cols) {
  int maxVal = accumulator[0][0];

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (accumulator[i][j] > maxVal) {
        maxVal = accumulator[i][j];
      }
    }
  }

  return maxVal;
}

int isNearZero(double value, double epsilon) {
  if (fabs(value) < epsilon || fabs(value - M_PI) < epsilon ||
      fabs(value - M_PI_2) < epsilon)
    return 1;
  return 0;
}

struct DetectedLines performHoughTransform(SDL_Surface *surface) {
  const double DIAGONAL =
      sqrt(surface->w * surface->w + surface->h * surface->h);

  const int RHO_MAX = (int)(2 * DIAGONAL) + 1;

  const int RHO_OFFSET = RHO_MAX / 2;

  int w = surface->w;
  int h = surface->h;
  int n = 50;
  // create pointer of lines
  struct Line *lines = calloc(n, sizeof(struct Line));

  // create accumulator
  int **accumulator = malloc(RHO_MAX * sizeof(int *));
  for (int i = 0; i < RHO_MAX; i++)
    accumulator[i] = calloc(theta_s, sizeof(int));

  if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0) {
    // Handle the error, perhaps log it or exit
    fprintf(stderr, "Could not lock surface: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  // Iterate over the pixels to collect votes
  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {
      Uint8 r, g, b;
      Uint32 pixelvalue = get_pixel(surface, x, y);
      SDL_GetRGB(pixelvalue, surface->format, &r, &g, &b);

      if (r == 255 && g == 255 && b == 255) { // or r>128
        for (int t = 0; t < theta_s; t++) {
          double currentTheta = t * theta;
          double rho = (x * cos(currentTheta)) + (y * sin(currentTheta));

          int rhoIndex = (int)(rho + RHO_OFFSET);

          if (rhoIndex >= 0 && rhoIndex < RHO_MAX) {
            accumulator[rhoIndex][t]++;
          }
        }
      }
    }
  }

  if (SDL_MUSTLOCK(surface)) {
    SDL_UnlockSurface(surface);
  }

  int maxval = accu_maxvalue(accumulator, RHO_MAX, theta_s);

  // The threshold will depend on your specific
  // application and image characteristics
  int lineindex = 0;
  const int THRESHOLD = maxval * 0.2;

  for (int r = 0; r < RHO_MAX; r++) {
    for (int t = 0; t < theta_s; t++) {
      // printf("%i",accumulator[r][t]);
      if (accumulator[r][t] > THRESHOLD) {
        double foundRho = (r - RHO_OFFSET);
        double foundTheta = t * theta;
        //	float epsilon = 0.01;

        int x1, y1, x2, y2;
        if (foundTheta < 0.01 ||
            fabs(foundTheta - M_PI) < 0.01) { // works perfectly //near 0 or pi

          // Line is approximately vertical
          x1 = x2 = foundRho / cos(foundTheta);
          y1 = 0;
          y2 = h;

          if (x1 < 0)
            x1 = x2 = 0;
          if (x1 >= w)
            x1 = x2 = w - 1;

        } else if (fabs(foundTheta - M_PI / 2) < 0.01) {
          // Line is approximately horizontal

          y1 = y2 = (foundRho / sin(foundTheta));
          x1 = 0;
          x2 = w;

          if (y1 < 0)
            y1 = y2 = 0;
          if (y1 >= h)
            y1 = y2 = h - 1;

          // x1=x2=y1=y2=0;

        } else {
          // diagonal
          x1 = 0;
          y1 = foundRho / sin(foundTheta);
          x2 = w;
          y2 = ((foundRho - x2 * cos(foundTheta)) / sin(foundTheta));

          x1 = x2 = y1 = y2 = 0;
        }

        if (lineindex == n) {
          n *= 2;
          lines = (struct Line *)realloc(lines, n * sizeof(struct Line));
        }

        lines[lineindex].start.x = x1;
        lines[lineindex].end.x = x2;
        lines[lineindex].start.y = y1;
        lines[lineindex].end.y = y2;
        lines[lineindex].theta = t;
        lines[lineindex].rho = foundRho;
        lineindex++;
      }
    }
  }
  for (int i = 0; i < RHO_MAX; i++) {
    free(accumulator[i]);
  }
  free(accumulator);

  struct Line *temp =
      (struct Line *)realloc(lines, lineindex * sizeof(struct Line));
  if (temp == NULL) {
    // Handle memory allocation failure
      free(lines);
  }
  lines = temp;

  // printf("%i",lineindex);
  struct DetectedLines result;
  result.lines = lines;
  result.count = lineindex;

  return result;

  // return lines;
}

// CALCULATE AVERAGE OF THE LINES

float distance(struct Point a, struct Point b) {
  return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

int thelinebelongs(struct Line l1, struct Line l2) {

  // if (fabs(l1.rho - l2.rho) <= rhoThreshold && fabs(l1.theta -
  // l2.theta) <= thetaThreshold)

  float startDistance = distance(l1.start, l2.start);
  float endDistance = distance(l1.end, l2.end);

  if (startDistance < startThreshold && endDistance < endThreshold)
    return 1;
  return 0;
}

void addtogroup(struct Line line, struct Linegroup *group) {
  group->average.start.x =
      (group->average.start.x * group->numlines + line.start.x) /
      (group->numlines + 1);
  group->average.start.y =
      (group->average.start.y * group->numlines + line.start.y) /
      (group->numlines + 1);
  group->average.end.x = (group->average.end.x * group->numlines + line.end.x) /
                         (group->numlines + 1);
  group->average.end.y = (group->average.end.y * group->numlines + line.end.y) /
                         (group->numlines + 1);
  group->average.theta = line.theta;
  group->average.rho = line.rho;
  group->numlines += 1;
}

int averagelines(struct Line *line, int len, struct Linegroup **group) {
  int n = 100;
  int ncurrent = 0;

  for (int i = 0; i < len; i++) {
    int loner = 1; // true = 1
    for (int j = 0; j < ncurrent; j++) {
      if (thelinebelongs(line[i], group[j]->average) == 1) {
        addtogroup(line[i], group[j]);
        loner = 0; // false
      }
    }
    if (loner == 1) {
      if (ncurrent == n) {
        n *= 2;
        group = realloc(group, n * sizeof(struct Linegroup *));

        if (!group) {
          fprintf(stderr, "Memory allocation failed.\n");
          exit(EXIT_FAILURE);
        }

        for (int j = ncurrent; j < n; j++) {
          group[j] = NULL;
        }
      }
      group[ncurrent] = calloc(1, sizeof(struct Linegroup));
      if (!group[ncurrent]) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
      }
      group[ncurrent]->average.start.x = line[i].start.x;
      group[ncurrent]->average.start.y = line[i].start.y;

      group[ncurrent]->average.end.x = line[i].end.x;
      group[ncurrent]->average.end.y = line[i].end.y;

      group[ncurrent]->average.theta = line[i].theta;
      group[ncurrent]->average.rho = line[i].rho;

      group[ncurrent]->numlines = 1;
      ncurrent++;
    }
  }
  return ncurrent;
}

void drawl(struct Line *line, int len, SDL_Renderer *renderer,
           SDL_Texture *texture) {
  struct Linegroup **group = calloc(len, sizeof(struct Linegroup *));
  // use len as it represents the number of lines
  if (!group) {
    fprintf(stderr, "Memory allocation failed.\n");
    exit(EXIT_FAILURE);
  }
  int nmax = averagelines(line, len, group);

  //  SDL_RenderClear(renderer);

  //	SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

  int render = SDL_RenderCopy(renderer, texture, NULL, NULL);
  if (render != 0)
    errx(EXIT_FAILURE, "%s", SDL_GetError());

  /*for(int i = 0; i < nmax; i++)
  {
          printf("start x %f y %f, end x%f y %f", group[i]->
                  average.start.x, group[i]->average.start.y,
                          group[i]->average.end.x, group[i]->
                          average.end.y);
          SDL_SetRenderDrawColor(renderer, 255, 0, 0,
                  SDL_ALPHA_OPAQUE); // Red color for the lines
          SDL_RenderDrawLine(renderer, group[i]->average.start.x,
                  group[i]->average.start.y,
                          group[i]->average.end.x, group[i]->
                          average.end.y);
  }*/
  SDL_RenderPresent(renderer);

  for (int i = 0; i < nmax; i++)
    free(group[i]);
  free(group);
}

struct DetectedLines averagearray(struct Line *Line, int len) {
  struct Linegroup **group = calloc(len, sizeof(struct Linegroup *));
  // use len as it represents the number of lines
  if (!group) {
    fprintf(stderr, "Memory allocation failed.\n");
    exit(EXIT_FAILURE);
  }
  int nmax = averagelines(Line, len, group);

  struct DetectedLines result;
  result.lines = calloc(nmax, sizeof(struct Line));
  result.count = nmax;
  // printf("nmax %i \n", nmax);

  for (int i = 0; i < nmax; i++) {
    // if (isPartOfSudoku(group,  nmax, group[i])) {

    result.lines[i].start.x = group[i]->average.start.x;
    result.lines[i].start.y = group[i]->average.start.y;
    result.lines[i].end.x = group[i]->average.end.x;
    result.lines[i].end.y = group[i]->average.end.y;
    result.lines[i].rho = group[i]->average.rho;
    result.lines[i].theta = group[i]->average.theta;

    // printf("nmax %i\n x %f",j , result.lines[j].start.x);

    //}

    free(group[i]);
  }
  free(group);

  return result;
}

// DETECT THE SQUARES

void horizontal_vertical_lines(struct Line *lines, int len,
                               struct Line *horizon, struct Line *vertical) {
  // thtea en degree
  int j = 0, k = 0;
  for (int i = 0; i < len; i++) {
    float t = lines[i].theta;
    // printf("t value %f \n",t);
    float foundTheta = t * theta;
    if (foundTheta < 0.01 || fabs(foundTheta - M_PI) < 0.01)
    // if ( (t>= -10 && t <= 10) || (t>= 170 && t<=190))

    {
      vertical[j] = lines[i];
      j++;
    } else if (fabs(foundTheta - M_PI / 2) < 0.01) {
      horizon[k] = lines[i];
      k++;
    }
  }
  // printf("len h %i, len v %i \n",j+1,k+1);
}

void sort_horizontal_lines(struct Line *horizon, int len) {
  int i, j;
  struct Line compare;

  for (i = 1; i < len; i++) {
    // printf("(%f,%f)",horizon[i].start.x,horizon[i].start.y);
    compare = horizon[i];
    j = i - 1;

    // Move elements that are greater
    // than key to one position ahead of their current position
    while (j >= 0 && horizon[j].start.y > compare.start.y) {
      horizon[j + 1] = horizon[j];
      j = j - 1;
    }
    horizon[j + 1] = compare;
  }

  /*	for(int k = 0; k <len; k++)
                  printf("(%f,%f) \n",horizon[k].start.x,horizon[k].start.y);
          printf("\n");*/
}

void sort_vertical_lines(struct Line *vertical, int len) {
  int i, j;
  struct Line compare;

  for (i = 1; i < len; i++) {
    compare = vertical[i];
    j = i - 1;

    // Move elements that are greater
    // than key to one position ahead of their current position
    while (j >= 0 && vertical[j].start.x > compare.start.x) {
      vertical[j + 1] = vertical[j];
      j = j - 1;
    }
    vertical[j + 1] = compare;
  }
}

void fillsquares(struct Line *vertical, struct Line *horizon,
                 struct Squares *squares, int len) {
  int nx = 0;

  // Iterate over each horizontal line
  for (int i = 0; i < len - 1; i++) {
    // For each horizontal line, iterate over each vertical line
    for (int j = 0; j < len - 1; j++) {

      squares[nx].topleft.y = horizon[i].start.y;
      squares[nx].topleft.x = vertical[j].start.x;

      squares[nx].topright.y = horizon[i].start.y;
      squares[nx].topright.x = vertical[j + 1].start.x;

      squares[nx].bottomleft.y = horizon[i + 1].start.y;
      squares[nx].bottomleft.x = vertical[j].start.x;

      squares[nx].bottomright.y = horizon[i + 1].start.y;
      squares[nx].bottomright.x = vertical[j + 1].start.x;
      nx++;
    }
  }
}
int has_white_neighbors(SDL_Surface *surface, int x, int y) {
  Uint32 white_pixel = SDL_MapRGB(surface->format, 255, 255, 255);
  for (int dx = -15; dx <= 15; dx++) {
    for (int dy = -15; dy <= 15; dy++) {
      if (dx == 0 && dy == 0)
        continue; // Skip the pixel itself
      if (x + dx < 0 || x + dx >= surface->w || y + dy < 0 ||
          y + dy >= surface->h)
        continue; // Check bounds
      Uint32 neighbor_pixel = get_pixel(surface, x + dx, y + dy);
      if (neighbor_pixel == white_pixel) {
        return 1; // White neighbor found
      }
    }
  }
  return 0; // No white neighbors
}
int check_square_edges(SDL_Surface *surface, struct Squares sq) {
  // Check top and bottom edges
  for (int x = sq.topleft.x; x <= sq.topright.x; x++) {
    if (has_white_neighbors(surface, x, sq.topleft.y) == 0) {
      return 0;
    }
    if (has_white_neighbors(surface, x, sq.topright.y) == 0) {
      return 0;
    }
  }
  for (int x = sq.bottomleft.x; x <= sq.bottomright.x; x++) {
    if (has_white_neighbors(surface, x, sq.bottomleft.y) == 0) {
      return 0;
    }
    if (has_white_neighbors(surface, x, sq.bottomright.y) == 0) {
      return 0;
    }
  }

  // Check left and right edges
  for (int y = sq.topleft.y; y <= sq.bottomleft.y; y++) {
    if (has_white_neighbors(surface, sq.topleft.x, y) == 0) {
      return 0;
    }
    if (has_white_neighbors(surface, sq.bottomleft.x, y) == 0) {
      return 0;
    }
  }
  for (int y = sq.topright.y; y <= sq.bottomright.y; y++) {
    if (has_white_neighbors(surface, sq.topright.x, y) == 0) {
      return 0;
    }
    if (has_white_neighbors(surface, sq.bottomright.x, y) == 0) {
      return 0;
    }
  }
  return 1;
}
struct Squares findbestsquare(SDL_Surface *original_image,
                              struct Line *vertical, struct Line *horizon,
                              int len) {
  struct Squares max_square;
  int max_size = -1;

  for (int h = 0; h < len - 1; ++h) {
    for (int v = 0; v < len - 1; ++v) {
      // Iterate from the farthest line to form larger squares first
      for (int hv = len - 1; hv > h; --hv) {
        for (int vv = len - 1; vv > v; --vv) {
          struct Squares s;

          s.topleft.x = vertical[v].start.x;
          s.topleft.y = horizon[h].start.y;
          s.topright.x = vertical[vv].start.x;
          s.topright.y = horizon[h].start.y;
          s.bottomleft.x = vertical[v].start.x;
          s.bottomleft.y = horizon[hv].start.y;
          s.bottomright.x = vertical[vv].start.x;
          s.bottomright.y = horizon[hv].start.y;

          float width = fabs(s.topright.x - s.topleft.x);
          float height = fabs(s.bottomleft.y - s.topleft.y);
          int size = width * height;

          if ((size > max_size) && fabs(height - width) < 10 &&
              check_square_edges(original_image, s)) {
            max_size = size;
            max_square = s;
          }
        }
      }
    }
  }

  return max_square;
}

void printvalues(struct Line *lines, int len, SDL_Surface *original_image) {
  struct Line *horizon = calloc(len / 2, sizeof(struct Line));
  struct Line *vertical = calloc(len / 2, sizeof(struct Line));

  if (!horizon || !vertical) {
    fprintf(stderr, "Memory allocation failed.\n");
    free(horizon);
    free(vertical);
    return;
  }

  horizontal_vertical_lines(lines, len, horizon, vertical);

  sort_horizontal_lines(horizon, len / 2);

  sort_vertical_lines(vertical, len / 2);

  int num_squares = (len / 2 - 1) * (len / 2 - 1);

  struct Squares *squares = calloc(num_squares, sizeof(struct Squares));

  /*	if (!squares) {
                  fprintf(stderr, "Memory allocation failed.\n");*/

  fillsquares(vertical, horizon, squares, len / 2);

/*  for (int i = 0; i < num_squares; i++) {
    printf("%i \n", i);
    printf("Top Left: (%f, %f)\n", squares[i].topleft.x, squares[i].topleft.y);
    printf("Top Right: (%f, %f)\n", squares[i].topright.x,
           squares[i].topright.y);
    printf("Bottom Right: (%f, %f)\n", squares[i].bottomright.x,
           squares[i].bottomright.y);
    printf("Bottom Left: (%f, %f)\n", squares[i].bottomleft.x,
           squares[i].bottomleft.y);
    printf("\n");
  }
  struct Squares ss = findbestsquare(original_image, vertical, horizon, len);
  printf("last big  \n");
  printf("Top Left: (%f, %f)\n", ss.topleft.x, ss.topleft.y);
  printf("Top Right: (%f, %f)\n", ss.topright.x, ss.topright.y);
  printf("Bottom Right: (%f, %f)\n", ss.bottomright.x, ss.bottomright.y);
  printf("Bottom Left: (%f, %f)\n", ss.bottomleft.x, ss.bottomleft.y);
  printf("\n");
  printf("t lf         %f,%f\n", ss.topleft.x, ss.topleft.y);
  printf("%f,%f\n", ss.bottomleft.x, ss.bottomleft.y);

  printf("t r          %f,%f\n", ss.bottomright.x, ss.bottomright.y);
  printf("%f,%f\n", ss.bottomleft.x, ss.bottomleft.y);
*/
  free(horizon);
  free(vertical);
  free(squares);
}

// Calculate the average  with and height of the  squares
void calculate_average_dimensions(struct Squares *squares, int num_squares,
                                  float *average_width, float *average_height) {

  // int n = 0;
  float total_width = 0, total_height = 0;

  for (int i = 0; i < num_squares; i++) {
    float width = squares[i].topright.x - squares[i].topleft.x;
    float height = squares[i].bottomleft.y - squares[i].topleft.y;

    // if (fabs(width - height) <10)
    //{
    total_width += width;
    total_height += height;

    //}
  }
  if (num_squares > 0) {
    *average_width = total_width / num_squares;
    *average_height = total_height / num_squares;
  } else {
    *average_width = 0;
    *average_height = 0;
  }
}

// sort squares in order
int compare_squares(const void *a, const void *b) {
  struct Squares *squareA = (struct Squares *)a;
  struct Squares *squareB = (struct Squares *)b;

  // First sort by top Y coordinate, then by left X coordinate
  if (squareA->topleft.y < squareB->topleft.y)
    return -1;
  if (squareA->topleft.y > squareB->topleft.y)
    return 1;
  if (squareA->topleft.x < squareB->topleft.x)
    return -1;
  if (squareA->topleft.x > squareB->topleft.x)
    return 1;

  return 0;
}

void sort_squares_horizontal(struct Squares *squares, int num_squares) {
  qsort(squares, num_squares, sizeof(struct Squares), compare_squares);
}

struct Squares *drawsquares(struct Line *lines, int len, struct Line *horizon,
                            struct Line *vertical) {
  horizontal_vertical_lines(lines, len, horizon, vertical);

  sort_horizontal_lines(horizon, len / 2);

  sort_vertical_lines(vertical, len / 2);
  //printf("sort\n");

  int num_squares = (len / 2 - 1) * (len / 2 - 1);


  struct Squares *sq = calloc(num_squares, sizeof(struct Squares));
  // printf("sq callo\n");

  /*	if (!squares) {
                  fprintf(stderr, "Memory allocation failed.\n");*/
  fillsquares(vertical, horizon, sq, len / 2);

  sort_squares_horizontal(sq, num_squares);

  return sq;

}

// Function to extract and save squares as images
void extract_and_save_squares(SDL_Surface *original_image,
                              struct Squares *squares, int num_squares,
                              struct Squares s) {
  const int target_width = 28;
  const int target_height = 28;

  float average_width, average_height;
  calculate_average_dimensions(squares, num_squares, &average_width,
                               &average_height);

  float big_square_width = fabs(s.topright.x - s.topleft.x);
  float big_square_height = fabs(s.bottomleft.y - s.topleft.y);

  float small_square_width = big_square_width / 9;
  float small_square_height = big_square_height / 9;

  int counter = 0;

  for (int row = 0; row < 9; row++) {
    for (int col = 0; col < 9; col++) {
      struct Squares small_square;

      small_square.topleft.x = s.topleft.x + col * small_square_width;
      small_square.topleft.y = s.topleft.y + row * small_square_height;

      small_square.bottomright.x = small_square.topleft.x + small_square_width;
      small_square.bottomright.y = small_square.topleft.y + small_square_height;

      // Create a new surface for each square
      SDL_Surface *square_surface = SDL_CreateRGBSurface(
          0, small_square_width, small_square_height,
          original_image->format->BitsPerPixel, original_image->format->Rmask,
          original_image->format->Gmask, original_image->format->Bmask,
          original_image->format->Amask);

      if (square_surface == NULL) {
        fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        continue; // Skip this square and move to the next
      }

      // Define the rectangle to be copied
      SDL_Rect square_rect;
      square_rect.x = small_square.topleft.x;
      square_rect.y = small_square.topleft.y;
      square_rect.w = small_square_width;
      square_rect.h = small_square_height;

      // Blit the square area from the original image to the new surface
      SDL_BlitSurface(original_image, &square_rect, square_surface, NULL);

      // Create a new surface for the resized square
      SDL_Surface *resized_surface = SDL_CreateRGBSurface(
          0, target_width, target_height, original_image->format->BitsPerPixel,
          original_image->format->Rmask, original_image->format->Gmask,
          original_image->format->Bmask, original_image->format->Amask);
      if (resized_surface == NULL) {
        fprintf(stderr, "SDL_CreateRGBSurface failed for resized surface: %s\n",
                SDL_GetError());
        SDL_FreeSurface(square_surface);
        continue;
      }

      // Blit the square surface to the resized surface with scaling
      SDL_BlitScaled(square_surface, NULL, resized_surface, NULL);

      // Save each square as an image
      char filename[64];
      sprintf(filename, "./images/square_%d.bmp", counter);

      if (SDL_SaveBMP(resized_surface, filename) != 0) {
        fprintf(stderr, "SDL_SaveBMP failed: %s\n", SDL_GetError());
      }
      counter++;

      // Free the square surface after saving
      SDL_FreeSurface(square_surface);
      SDL_FreeSurface(resized_surface);
    }
  }
}

/// AUTOMATIC ROTATION

// HOUGH
struct DetectedLines auto_performHoughTransform(SDL_Surface *surface) {
  const double DIAGONAL =
      sqrt(surface->w * surface->w + surface->h * surface->h);

  const int RHO_MAX = (int)(2 * DIAGONAL) + 1;

  const int RHO_OFFSET = RHO_MAX / 2;

  int w = surface->w;
  int h = surface->h;
  int n = 50;
  // create pointer of lines
  struct Line *lines = calloc(n, sizeof(struct Line));

  // create accumulator
  int **accumulator = malloc(RHO_MAX * sizeof(int *));
  for (int i = 0; i < RHO_MAX; i++)
    accumulator[i] = calloc(theta_s, sizeof(int));

  if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0) {
    // Handle the error, perhaps log it or exit
    fprintf(stderr, "Could not lock surface: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  // Iterate over the pixels to collect votes
  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {
      Uint8 r, g, b;
      Uint32 pixelvalue = get_pixel(surface, x, y);
      SDL_GetRGB(pixelvalue, surface->format, &r, &g, &b);

      if (r == 255 && g == 255 && b == 255) { // or r>128
        for (int t = 0; t < theta_s; t++) {
          double currentTheta = t * theta;
          double rho = (x * cos(currentTheta)) + (y * sin(currentTheta));

          int rhoIndex = (int)(rho + RHO_OFFSET);

          if (rhoIndex >= 0 && rhoIndex < RHO_MAX) {
            accumulator[rhoIndex][t]++;
          }
        }
      }
    }
  }

  if (SDL_MUSTLOCK(surface)) {
    SDL_UnlockSurface(surface);
  }

  int maxval = accu_maxvalue(accumulator, RHO_MAX, theta_s);

  // The threshold will depend on your specific
  // application and image characteristics
  int lineindex = 0;
  const int THRESHOLD = maxval * 0.2;

  for (int r = 0; r < RHO_MAX; r++) {
    for (int t = 0; t < theta_s; t++) {
      // printf("%i",accumulator[r][t]);
      if (accumulator[r][t] > THRESHOLD) {
        double foundRho = (r - RHO_OFFSET);
        double foundTheta = t * theta;
        //  float epsilon = 0.01;

        int x1, y1, x2, y2;
        if (foundTheta < 0.01 ||
            fabs(foundTheta - M_PI) < 0.01) { // works perfectly //near 0 or pi

          // Line is approximately vertical
          x1 = x2 = foundRho / cos(foundTheta);
          y1 = 0;
          y2 = h;

          if (x1 < 0)
            x1 = x2 = 0;
          if (x1 >= w)
            x1 = x2 = w - 1;

        } else if (fabs(foundTheta - M_PI / 2) < 0.01) {
          // Line is approximately horizontal

          y1 = y2 = (foundRho / sin(foundTheta));
          x1 = 0;
          x2 = w;

          if (y1 < 0)
            y1 = y2 = 0;
          if (y1 >= h)
            y1 = y2 = h - 1;

          // x1=x2=y1=y2=0;

        } else {
          // diagonal
          x1 = 0;
          y1 = foundRho / sin(foundTheta);
          x2 = w;
          y2 = ((foundRho - x2 * cos(foundTheta)) / sin(foundTheta));

          //  x1=x2=y1=y2=0;

          if (lineindex == n) {
            n *= 2;
            lines = (struct Line *)realloc(lines, n * sizeof(struct Line));
          }

          lines[lineindex].start.x = x1;
          lines[lineindex].end.x = x2;
          lines[lineindex].start.y = y1;
          lines[lineindex].end.y = y2;
          lines[lineindex].theta = t;
          lines[lineindex].rho = foundRho;
          lineindex++;
        }
      }
    }
  }
  for (int i = 0; i < RHO_MAX; i++) {
    free(accumulator[i]);
  }
  free(accumulator);

  struct Line *temp =
      (struct Line *)realloc(lines, lineindex * sizeof(struct Line));
  if (temp == NULL) {
    // Handle memory allocation failure
    free(lines);
  }
  lines = temp;

  // printf("%i",lineindex);
  struct DetectedLines result;
  result.lines = lines;
  result.count = lineindex;

  return result;

  // return lines;
}

double calculate_angle(struct DetectedLines result) {
  struct Line *lines = result.lines;
  int count = result.count;

  int theta_bins[360] = {0};

  // Count occurrences of each theta in bins
  for (int i = 0; i < count; ++i) {
    int bin_index = (int)(lines[i].theta) % 360;

    theta_bins[bin_index]++;
  }

  // Find the theta bin with the highest count
  int most_common_theta = 0, max_count = 0;
  for (int i = 0; i < 360; ++i) {
    if (theta_bins[i] > max_count) {
      max_count = theta_bins[i];
      most_common_theta = i;
    }
  }

  // if (most_common_theta < 90) most_common_theta +=90;
  return (double)(most_common_theta - 90);
}

// rotate the image with corrersponding angle
SDL_Surface *RotateImage(SDL_Surface *image, double angledegree) {
  // angledegree = fmod(angledegree, 360.0);
    SDL_LockSurface(image);

  int w1 = image->w;
  int h1 = image->h;
  if (angledegree == 0) {
    return SDL_ConvertSurface(image, image->format, 0); // Create a copy
  }

  float rad = angledegree * M_PI / 180.0;

  double coco = cos(rad);
  double sisi = sin(rad);

  int w2 = fabs(coco * w1) + fabs(sisi * h1); // x' = xcos + ysin
  int h2 = fabs(sisi * w1) + fabs(coco * h1);

  SDL_Surface *rot = SDL_CreateRGBSurface(0, w2, h2, 32, 0, 0, 0, 0);
    SDL_LockSurface(rot);


  for (int y = 0; y < h2; y++) {
    for (int x = 0; x < w2; x++) {
      int middlex = w1 / 2; // center coordinates of
                            // the source image
      int middley = h1 / 2;

      // with the distance to the center for each pixe
      //-> trigonometric calculations can be used
      //(the rotation matrix)

      int distancex = x - w2 / 2; // distance of
                                  // current pixel
      // from the center of the destination image in the x direction.
      int distancey = y - w2 / 2;

      // the corresponding pixel in the source image
      int truex = middlex + distancex * coco - distancey * sisi;
      int truey = middley + distancey * coco + distancex * sisi;

      if (truex >= 0 && truey >= 0 && truex < w1 && truey < h1)
        ((Uint32 *)rot->pixels)[y * w2 + x] =
            ((Uint32 *)image->pixels)[truey * w1 + truex];
      else
        ((Uint32 *)rot->pixels)[y * w2 + x] = 0x000000;
      // transparent sdl rgb
    }
  }

  SDL_UnlockSurface(image);
  SDL_UnlockSurface(rot);

  // memcpy(image->pixels, rot->pixels, w * h * sizeof(Uint32))

  return rot;
}
