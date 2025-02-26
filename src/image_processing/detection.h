
#pragma once
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "hough.h"
SDL_Surface* load_image(const char* path);
void update_render_scale(SDL_Renderer* renderer, int new_width, int new_height);
void drawk(SDL_Renderer *renderer, SDL_Texture *texture, struct Line *lines,
           int num_lines);
void draw_squares(SDL_Renderer *renderer, SDL_Texture *texture,
                  struct Squares *squares, int num_lines, struct Squares s);
void draw_h_v(SDL_Renderer *renderer, SDL_Texture *texture,
              struct Line *horizon, int num_lines, struct Line *vertical);
void event_loop_image(SDL_Renderer *renderer, SDL_Texture *t_image);
void event_loop_image_l(SDL_Renderer *renderer, SDL_Texture *t_image,
                        struct Line *line, int n, SDL_Surface* save);
void event_loop_image_test_averagelines(SDL_Renderer *renderer,
                                        SDL_Texture *t_image,
                                        struct Line *lines, int numline);
void event_loop_image_test_sq(SDL_Renderer *renderer, SDL_Texture *t_image,
                              struct Squares *squares, int num,
                              struct Squares s,SDL_Surface* save);

void event_loop_image_l2(SDL_Renderer *renderer, SDL_Texture *t_image,
                        struct Line *line, int n, SDL_Surface* save);

