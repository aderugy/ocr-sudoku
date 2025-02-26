#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

void get_tab(const char *path, double *pixels) {
    SDL_Surface *surface = IMG_Load(path);
    if (surface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return;
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }

    int pixelIndex = 0;
    int bpp = surface->format->BytesPerPixel; // Bytes per pixel

    for (int y = 0; y < surface->h; ++y) {
        Uint8 *row = (Uint8 *)surface->pixels + y * surface->pitch;
        for (int x = 0; x < surface->w; ++x) {
            Uint32 pixelColor;
            memcpy(&pixelColor, row + x * bpp, bpp);

            Uint8 r, g, b;
            SDL_GetRGB(pixelColor, surface->format, &r, &g, &b);
            Uint8 grayscale = (Uint8)(0.3 * r + 0.59 * g + 0.11 * b);

            pixels[pixelIndex] = (grayscale > 128) ? 1 : 0;
            pixelIndex++;
        }
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }

    SDL_FreeSurface(surface);
    IMG_Quit();
    SDL_Quit();
}
