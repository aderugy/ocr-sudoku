

#ifndef IMAGE_H
#define IMAGE_H

void surface_to_grayscale(SDL_Surface* surface);
Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat* format);
void surface_to_blackwhite(SDL_Surface* surface);
//Uint8 calculate_local_threshold(SDL_Surface* surface, int x, int y,
//
//int neighborhood_size);
Uint32* intergral_image(SDL_Surface* surface);
Uint8 fast_local_threshold(Uint32* integral, int x, int y,
        int neighborhood_size, int width, int height);

void surface_to_contrast(SDL_Surface* surface, float contrast);
Uint32 pixel_to_contrast(Uint32 pixel_color, SDL_PixelFormat* format,
        float contrast);
float** generate_Kernel(int ksize, float sigma);
void applyblur (SDL_Surface * image, float** kernel, int kernelsize,
        SDL_Surface* filteredimage);
void surface_to_reducenoise(SDL_Surface* surface);
void surface_to_inverse(SDL_Surface* surface);
//bool White(Uint32 pixel, SDL_PixelFormat* format);
void dilation(SDL_Surface* image);
void erosion(SDL_Surface* image);
void Canny_edge_result (SDL_Surface* image);

#endif
