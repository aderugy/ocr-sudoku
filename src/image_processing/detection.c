#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "hough.h"
#include "detection.h"

SDL_Surface *load_image(const char *path) {
    SDL_Surface *is = IMG_Load(path);
    if (is == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_Surface *fs = SDL_ConvertSurfaceFormat(is, SDL_PIXELFORMAT_RGB888, 0);
    if (fs == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_FreeSurface(is);
    return fs;
}

void update_render_scale(SDL_Renderer *renderer, int new_width,
        int new_height) {
    int original_image_width = 0;
    int original_image_height = 0;

    float scale_x = (float)new_width / original_image_width;
    float scale_y = (float)new_height / original_image_height;
    SDL_RenderSetScale(renderer, scale_x, scale_y);
}

void draw(SDL_Renderer *renderer, SDL_Texture *texture) {
    int render = SDL_RenderCopy(renderer, texture, NULL, NULL);
    if (render != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    SDL_RenderPresent(renderer);
}

void drawk(SDL_Renderer *renderer, SDL_Texture *texture, struct Line *lines,
        int num_lines) {
    int render = SDL_RenderCopy(renderer, texture, NULL, NULL);
    if (render != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    //	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //	SDL_RenderClear(renderer);

    // Draw detected lines
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // Red color for lines
    for (int i = 0; i < num_lines; i++) {
        SDL_RenderDrawLine(renderer, lines[i].start.x, lines[i].start.y,
                lines[i].end.x, lines[i].end.y);
    }
    // SDL_RenderPresent(renderer);

    SDL_RenderPresent(renderer);
}

void draw_squares(SDL_Renderer *renderer, SDL_Texture *texture,
        struct Squares *squares, int num_lines, struct Squares s) {
    int render = SDL_RenderCopy(renderer, texture, NULL, NULL);
    if (render != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    //	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //	SDL_RenderClear(renderer);

    // Draw detected lines
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // Red color for lines
    for (int i = 0; i < num_lines; i++) {
        //	SDL_RenderDrawLine(renderer, 100,500, 500,500);

        SDL_RenderDrawLine(renderer, squares[i].topleft.x,
			squares[i].topleft.y,
                squares[i].topleft.x + 10, squares[i].topleft.y + 10);
        SDL_RenderDrawLine(renderer,
			squares[i].topright.x, squares[i].topright.y,
                squares[i].topright.x + 10, squares[i].topright.y + 10);
        SDL_RenderDrawLine(renderer, squares[i].bottomright.x,
                squares[i].bottomright.y, squares[i].bottomright.x + 10,
                squares[i].bottomright.y + 10);

        SDL_RenderDrawLine(renderer, squares[i].bottomleft.x,
                squares[i].bottomleft.y, squares[i].bottomleft.x + 10,
                squares[i].bottomleft.y + 10);
    }
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 0);

    // Coordinates for the square

    int x1 = s.topleft.x, y1 = s.topleft.y;         // Top Left
    int x2 = s.topright.x, y2 = s.topright.y;       // Top Right
    int x3 = s.bottomright.x, y3 = s.bottomright.y; // Bottom Right
    int x4 = s.bottomleft.x, y4 = s.bottomleft.y;   // Bottom Left

    // Draw the square
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    SDL_RenderDrawLine(renderer, x2, y2, x3, y3);
    SDL_RenderDrawLine(renderer, x3, y3, x4, y4);
    SDL_RenderDrawLine(renderer, x4, y4, x1, y1);

    // SDL_RenderPresent(renderer);

    SDL_RenderPresent(renderer);
}

void draw_h_v(SDL_Renderer *renderer, SDL_Texture *texture,
        struct Line *horizon, int num_lines, struct Line *vertical) {
    int render = SDL_RenderCopy(renderer, texture, NULL, NULL);
    if (render != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    //	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //	SDL_RenderClear(renderer);

    // Draw detected lines
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // Red color for lines
    for (int i = 0; i < num_lines; i++) {
        SDL_RenderDrawLine(renderer, vertical[i].start.x, vertical[i].start.y,
                vertical[i].end.x, vertical[i].end.y);
        SDL_RenderDrawLine(renderer, horizon[i].start.x, horizon[i].start.y,
                horizon[i].end.x, horizon[i].end.y);
    }
    // SDL_RenderPresent(renderer);

    SDL_RenderPresent(renderer);
}

void event_loop_image(SDL_Renderer *renderer, SDL_Texture *t_image) {
    SDL_Event event;

    draw(renderer, t_image);
    while (1) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                return;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {

                    draw(renderer, t_image);
                    break;
                }
        }
    }
}
void event_loop_image_l(SDL_Renderer* renderer, SDL_Texture* t_image,
        struct Line* line, int n, SDL_Surface* saveSurface)
{
    // Draw on the renderer
    draw(renderer, t_image);
    drawk(renderer, t_image, line, n);
    SDL_RenderPresent(renderer); // Update the screen with the rendering

    // Save the rendered content to an SDL_Surface
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
                         saveSurface->pixels, saveSurface->pitch);
    
    // Save the SDL_Surface as an image file
    IMG_SaveJPG(saveSurface, "./images/hough.jpg", 100);
}


void event_loop_image_l2(SDL_Renderer* renderer, SDL_Texture* t_image,
        struct Line* line, int n, SDL_Surface* saveSurface)
{
    // Draw on the renderer
    draw(renderer, t_image);
    drawk(renderer, t_image, line, n);
    SDL_RenderPresent(renderer); // Update the screen with the rendering

    // Save the rendered content to an SDL_Surface
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
                         saveSurface->pixels, saveSurface->pitch);
    
    // Save the SDL_Surface as an image file
    IMG_SaveJPG(saveSurface, "./images/hough_average.jpg", 100);
}





void event_loop_image_test_averagelines(SDL_Renderer *renderer,
        SDL_Texture *t_image,
        struct Line *lines, int numline) {
    SDL_Event event;

    draw(renderer, t_image);
    while (1) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                return;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // drawl(renderer, t_image, lines);
                    update_render_scale(renderer, event.window.data1,
				    event.window.data2);
                    drawl(lines, numline, renderer, t_image);
                    break;
                }
        }
    }
}

void event_loop_image_test_sq(SDL_Renderer *renderer, SDL_Texture *t_image,
        struct Squares *squares, int num,
        struct Squares s, SDL_Surface *saveSurface) {
    SDL_Event event;

    draw(renderer, t_image);
    draw_squares(renderer, t_image, squares, num, s);

   SDL_RenderPresent(renderer); // Update the screen with the rendering

    // Save the rendered content to an SDL_Surface
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
                         saveSurface->pixels, saveSurface->pitch);
    
    // Save the SDL_Surface as an image file
    IMG_SaveJPG(saveSurface, "./images/drawsquares.jpg", 100);
}




