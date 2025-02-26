#pragma once

#include <gtk/gtk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./image_processing/image.h"
#include "./image_processing/processing.h"
// square_detection

#include "./image_processing/hough.h"
#include "./image_processing/detection.h"

void image_solve(){
    network *n = import_network("./networks/Final.nw");
    unsigned int a[81];

    for (int i = 0; i < 81; ++i) {
      char path[100];
      snprintf(path, sizeof(path), "./images/square_%d.bmp", i);
      double *pixels = malloc(784 * sizeof(double));
      get_tab(path, pixels);
      feed_forward(n, pixels);
      a[i] = read_output(n);
      free(pixels);
    }
    //    char* finalString = malloc(100);
    // snprintf(finalString, 100,"./data/grid_0%d\n", 1);
    FILE *f = fopen("./grids/grid_01", "w");
    //Just keep grid_01 is possible, but better if you chenge the name
    for (size_t index_dim = 0; index_dim < 9; index_dim++) {
      if (index_dim % 3 == 0 && index_dim != 0)
        fprintf(f, "\n");
      for (size_t j = 0; j < 9; ++j) {
        int c = a[index_dim * 9 + j];
        if (j % 3 == 0 && j != 0) {
          if (c <= 9 && c > 0)
            fprintf(f, " %d", a[index_dim * 9 + j]);
          else
            fprintf(f, " %c", '.');
        } else if (j % 8 == 0 && j != 0) {
          if (c <= 9 && c > 0)
            fprintf(f, "%d\n", a[index_dim * 9 + j]);
          else
            fprintf(f, "%c\n", '.');
        } else {

          if (c <= 9 && c > 0)
            fprintf(f, "%d", a[index_dim * 9 + j]);
          else
            fprintf(f, "%c", '.');
        }
      }
    }
    fclose(f);
    free_network(n);

    // SOLVER

    size_t dim = 9;
    unsigned int **FinalGrid = allocGrid(dim);

    char *r = "./grids/grid_01";
    gridReader(dim, FinalGrid, r);
    solve(FinalGrid, 0, 0, dim);
    char *res;
    // asprintf(&res, "%s%s", finalString, ".result");

    // free(finalString);

    gridWriter(dim, FinalGrid, "./grids/grid_01.result");
    freeGrid(FinalGrid, dim);
    printf("generated result grid in data");
}


void process_all(SDL_Renderer *renderer, SDL_Surface *surface){
    surface_to_grayscale(surface);
    IMG_SaveJPG(surface, "./images/grayscale.jpg", 100);

    surface_to_contrast(surface, 0.1);
    IMG_SaveJPG(surface, "./images/contrast.jpg", 100);

    surface_to_reducenoise(surface);
    IMG_SaveJPG(surface, "./images/reducenoise.jpg", 100);

    surface_to_blackwhite(surface);
    IMG_SaveJPG(surface, "./images/t.jpg", 100);

    surface_to_inverse(surface);
    IMG_SaveJPG(surface, "./images/inverse.jpg", 100);

    dilation(surface);
    IMG_SaveJPG(surface, "./images/dilation.jpg", 100);

    erosion(surface);
    IMG_SaveJPG(surface, "./images/erosion.jpg", 100);

    Canny_edge_result(surface);
    IMG_SaveJPG(surface, "./images/canny.jpg", 100);

}


void process_autorot(char *path) {

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		errx(EXIT_FAILURE, "%s", SDL_GetError());

	SDL_Window *window =
		SDL_CreateWindow("Dynamic Fractal Canopy", 0, 0, 400, 400,
				SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == NULL)
		errx(EXIT_FAILURE, "%s", SDL_GetError());

	SDL_Renderer *renderer =
		SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL)
		errx(EXIT_FAILURE, "%s", SDL_GetError());

	SDL_Surface *surface = load_image(path);
	if (surface == NULL)
		errx(EXIT_FAILURE, "%s", SDL_GetError());

	int w = surface->w;
	int h = surface->h;
	SDL_SetWindowSize(window, w, h);

	process_all(renderer, surface);
	/*SDL_Surface *ex = IMG_Load("./images/canny.jpg");

	if (!ex) {
		errx(EXIT_FAILURE, "Unable to load image: %s", IMG_GetError());
	}*/

	struct DetectedLines detected = auto_performHoughTransform(surface);

	double angle = calculate_angle(detected);

	SDL_Surface *im = RotateImage(surface, angle);
	IMG_SaveJPG(im, "./images/autorot.jpg", 100);
	//SDL_FreeSurface(ex); 

	SDL_FreeSurface(surface); // Now freeing the surface after extraction
	SDL_FreeSurface(im);

	free(detected.lines);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();



	SDL_Quit();

}

void process_image(char *path) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    errx(EXIT_FAILURE, "%s", SDL_GetError());

  SDL_Window *window =
      SDL_CreateWindow("Dynamic Fractal Canopy", 0, 0, 400, 400,
                       SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (window == NULL)
    errx(EXIT_FAILURE, "%s", SDL_GetError());

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (renderer == NULL)
    errx(EXIT_FAILURE, "%s", SDL_GetError());

  SDL_Surface *surface = load_image(path);
  if (surface == NULL)
    errx(EXIT_FAILURE, "%s", SDL_GetError());

  int w = surface->w;
  int h = surface->h;
  SDL_SetWindowSize(window, w, h);

  process_all(renderer, surface);
  SDL_Surface *extraction_surface = IMG_Load("./images/erosion.jpg");

  if (!extraction_surface) {
      errx(EXIT_FAILURE, "Unable to load image: %s", IMG_GetError());
  }

  //    SDL_Surface *canny = IMG_Load("canny.jpg");
  //   if (!extraction_surface) {
  //   errx(EXIT_FAILURE, "Unable to load image: %s", IMG_GetError());
  //}
  struct DetectedLines detected = performHoughTransform(surface);
  struct Line *lin = detected.lines;
  int num = detected.count;



  SDL_Texture *grayscale_texture =
	  SDL_CreateTextureFromSurface(renderer, surface);
  if (grayscale_texture == NULL)
	  errx(EXIT_FAILURE, "%s", SDL_GetError());

  SDL_Surface *saveSurface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  if (saveSurface == NULL)
	  errx(EXIT_FAILURE, "Failed to create surface for saving: %s",
			  SDL_GetError());


  event_loop_image_l(renderer, grayscale_texture, lin, num, saveSurface);
  SDL_FreeSurface(saveSurface); // Free the saveSurface


  struct DetectedLines d2 = averagearray(lin, num);
  struct Line *lines = d2.lines;
  int num_lines = d2.count;

  SDL_Surface *saveSurface2 = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  if (saveSurface2 == NULL)
	  errx(EXIT_FAILURE, "Failed to create surface for saving: %s", 
			  SDL_GetError());


  event_loop_image_l2(renderer, grayscale_texture, lines, num_lines, 
		  saveSurface2);
  SDL_FreeSurface(saveSurface2); // Free the saveSurface


  struct Line *horizon = calloc(num_lines / 2, sizeof(struct Line));
  struct Line *vertical = calloc(num_lines / 2, sizeof(struct Line));

  if (!horizon || !vertical) {
      fprintf(stderr, "Memory allocation failed.\n");
      free(horizon);
      free(vertical);
  }

  struct Squares *sq = drawsquares(lines, num_lines, horizon, vertical);
  struct Squares s =
      findbestsquare(surface, vertical, horizon, num_lines / 2);

 SDL_Surface *saveSurface3 = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  if (saveSurface3 == NULL)
	  errx(EXIT_FAILURE, "Failed to create surface for saving: %s",
			  SDL_GetError());



  event_loop_image_test_sq(renderer, grayscale_texture, sq,
          (num_lines / 2 - 1) * (num_lines / 2 - 1), s, saveSurface3);
  SDL_FreeSurface(saveSurface3); // Free the saveSurface


  extract_and_save_squares(extraction_surface, sq,
          (num_lines / 2 - 1) * (num_lines / 2 - 1), s);

  free(lines);
  free(lin);
  free(horizon);
  free(vertical);
  free(sq);
  SDL_DestroyTexture(grayscale_texture);
  SDL_FreeSurface(extraction_surface);
  SDL_FreeSurface(surface); // Now freeing the surface after extraction

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
