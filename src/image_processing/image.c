#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "detection.h"


// pixel_color: Color of the pixel to convert in the RGB format.
// format: Format of the pixel used by the surface.
Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat* format)
{
    Uint8 r, g, b;
    SDL_GetRGB(pixel_color, format, &r, &g, &b);

    double average = 0.3*r + 0.59*g + 0.11*b;
    return SDL_MapRGB(format, average, average, average);
}

void surface_to_grayscale(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    if (pixels == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;

    if (format == NULL || pixels == NULL || SDL_LockSurface(surface) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    for (int i = 0; i < len; i++)
    {
        pixels[i] = pixel_to_grayscale(pixels[i], format);
    }

    SDL_UnlockSurface(surface);
}


void surface_to_inverse(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    if (pixels == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;

    if (format == NULL || pixels == NULL || SDL_LockSurface(surface) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    for (int i = 0; i < len; i++)
    {
        Uint8 r, g, b;
        SDL_GetRGB(pixels[i], format, &r, &g, &b);
        if (r == 255)
            r =0;
        else{
            r = 255;
        }
        if (g == 255)
            g =0;
        else{
            g = 255;
        }
        if (b == 255)
            b = 0;
        else{
            b = 255;
        }

        pixels[i] = SDL_MapRGB(format, r, g, b);

    }

    SDL_UnlockSurface(surface);
}





Uint32* integral_image(SDL_Surface* surface) {
    Uint32* integral = malloc(surface->w * surface->h * sizeof(Uint32));
    if (!integral) {
        errx(EXIT_FAILURE, "Memory allocation for integral image failed");
    }

    SDL_LockSurface(surface);

    for (int y = 0; y < surface->h; y++) {
        Uint32 sum = 0;
        for (int x = 0; x < surface->w; x++) {
            Uint32 pixel = ((Uint32*)surface->pixels)[y * surface->w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);
            Uint8 average = (r + g + b) / 3;

            sum += average;
            if (y == 0) {
                integral[y * surface->w + x] = sum;
            } else {
                integral[y * surface->w + x] = integral[(y - 1) *
                    surface->w + x] + sum;
            }
        }
    }

    SDL_UnlockSurface(surface);
    return integral;
}

Uint8 fast_local_threshold(Uint32* integral, int x, int y,
        int neighborhood_size, int width, int height) {
    int side = neighborhood_size / 2;
    int x1 = x - side - 1;
    int y1 = y - side - 1;
    int x2 = x + side;
    int y2 = y + side;

    x1 = (x1 < 0) ? 0 : x1;
    y1 = (y1 < 0) ? 0 : y1;
    x2 = (x2 >= width) ? width - 1 : x2;
    y2 = (y2 >= height) ? height - 1 : y2;

    int count = (x2 - x1) * (y2 - y1);

    Uint32 sum = integral[y2 * width + x2] - integral[y1 * width + x2] -
        integral[y2 * width + x1] + integral[y1 * width + x1];
    return (Uint8)(sum / count);
}




void surface_to_blackwhite(SDL_Surface* surface) {
    int neighborhood_size = 20;
    if (neighborhood_size % 2 == 0) neighborhood_size++;

    SDL_Surface* copy = SDL_ConvertSurfaceFormat(surface,
            SDL_PIXELFORMAT_RGB888, 0);
    if (!copy) {
        errx(EXIT_FAILURE, "Unable to create surface copy: %s", SDL_GetError());
    }

    Uint32* integral = integral_image(copy);

    SDL_LockSurface(surface);
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint8 local_threshold = fast_local_threshold(integral, x, y,
                    neighborhood_size, copy->w, copy->h);

            Uint32 pixel = ((Uint32*)surface->pixels)[y * surface->w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);
            Uint8 average = (r + g + b) / 3;

            if (average < local_threshold) {
                ((Uint32*)surface->pixels)[y * surface->w + x] =
                    SDL_MapRGB(surface->format, 0, 0, 0);
            } else {
                ((Uint32*)surface->pixels)[y * surface->w + x] =
                    SDL_MapRGB(surface->format, 255, 255, 255);
            }
        }
    }

    SDL_UnlockSurface(surface);

    SDL_FreeSurface(copy);
    free(integral);
}



// Adjusts the contrast of a single pixel.
//
// pixel_color: Color of the pixel to adjust in the RGB format.
// format: Format of the pixel used by the surface.
// contrast: Contrast adjustment factor (e.g., 1.5 for increasing contrast).
Uint32 pixel_to_contrast(Uint32 pixel_color, SDL_PixelFormat* format,
        float contrast)
{
    Uint8 r, g, b;
    SDL_GetRGB(pixel_color, format, &r, &g, &b);

    float shifted_contrast_scale = (contrast - 1.0) * 128.0; //1

    // Adjust contrast
    int new_r = r + (int)((r - 128) * shifted_contrast_scale / 128.0);
    int new_g = g + (int)((g - 128) * shifted_contrast_scale / 128.0);
    int new_b = b + (int)((b - 128) * shifted_contrast_scale / 128.0);

    // Clamp the values to the [0, 255] range
    new_r = new_r > 255 ? 255 : new_r < 0 ? 0 : new_r;
    new_g = new_g > 255 ? 255 : new_g < 0 ? 0 : new_g;
    new_b = new_b > 255 ? 255 : new_b < 0 ? 0 : new_b;

    // Return the new color
    return SDL_MapRGB(format, new_r, new_g, new_b);

}


// Adjusts the contrast of an SDL surface.
//
// surface: The SDL surface to adjust.
// contrast: Contrast adjustment factor (e.g., 1.5 for increasing contrast).
void surface_to_contrast(SDL_Surface* surface, float contrast)
{
    Uint32* pixels = surface->pixels;
    if (pixels == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;
    if (format == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    int i = 0;
    SDL_LockSurface(surface);

    while (i < len) {
        pixels[i] = pixel_to_contrast(pixels[i], format, contrast);
        i++;
    }

    SDL_UnlockSurface(surface);
}


float** generate_Kernel(int ksize, float sigma)
{
    float sum = 0.0;

    int center = ksize /2;


    //The kernel matrix is a double pointer
    //we assign a specified amount of Memory
    //for an arvray of size ksize of type floats

    float** kernel = (float**)malloc(ksize * sizeof(float*));

    for (int i = 0; i < ksize; i++) {
        kernel[i] = (float*)calloc(ksize, sizeof(float));} //malloc

    for (int y = 0; y < ksize; y++) {
        for (int x = 0; x < ksize; x++) {

            //x and y must be coordinates such that 0
            //is in the center of the kernel
            int mx = x - center;
            int my = y - center;
	    if (mx >=0 && my >=0){
            double tmp = -(mx * mx + my * my) / (2 * sigma * sigma);

            kernel[y][x] =  1. / (2 * M_PI * sigma * sigma) * exp(tmp);
            sum += kernel[y][x];
	    }
        }
    }

    //Normalizing the kernel -> ensures no undesirable change in the
    //image's brightness (the filter behaves consistently).
    for (int y = 0; y < ksize; y++) {
        for (int x = 0; x < ksize; x++) {
            kernel[y][x] /= sum;
        }
    }
    return kernel;
}




void applyblur (SDL_Surface * image, float** kernel, int kernelsize,
		SDL_Surface* filteredimage)
{
    int imageWidth = image->w;
    int imageHeight = image->h;

    //filteredimage =  a separate output image, with the same
    //dimensions as the original image

    int center = kernelsize /2;


    //iterating through each pixel in the input image
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {

            float finalr = 0.0;
            float finalb = 0.0;
            float finalg = 0.0;

            //For each pixel, we multiply the pixel values in the image with
            //the corresponding values in the kernel and accumulates the
            //results.
            for (int ky = 0; ky < kernelsize; ky++) {
                for (int kx = 0; kx < kernelsize; kx++) {



                    int Xf = x + kx - center;
                    int Yf = y + ky - center;

                    //check if we are outside of the array
                    if (Xf >= 0 && Xf < imageWidth && Yf < imageHeight &&
                            Yf >=0) {
                        Uint32* pixelinitial = (Uint32*)image->pixels;


                        Uint32 pi = pixelinitial[Yf * imageWidth + Xf];

                        Uint8 r,g,b,a;
                        SDL_GetRGBA(pi, image->format, &r, &g, &b, &a);

                        finalr += kernel[kx][ky] * r;
                        finalb += kernel[kx][ky] *b;
                        finalg += kernel[kx][ky] *g;
                    }
                    //if we are outside of the image boundaries,
                    //the values are treated as being zero.
                }
            }
            Uint32 newPixelValue = SDL_MapRGBA(filteredimage->format,
                    (Uint8)finalr, (Uint8)finalg, (Uint8)finalb, 255);
            ((Uint32*)filteredimage->pixels)[y * imageWidth + x] =
                newPixelValue;


        }
    }
}





void surface_to_reducenoise(SDL_Surface* surface)
{
    // Define the kernel size and sigma for Gaussian blur
    int kernelSize = 5;
    float sigma = 15.0; //photo 2 = 10

    // Generate the Gaussian kernel
    float** kernel = generate_Kernel(kernelSize, sigma);

    SDL_Surface* outputimage = SDL_CreateRGBSurface(0, surface->w,
            surface->h, 32, 0, 0, 0, 0);
    if (outputimage == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());


    applyblur (surface, kernel, kernelSize, outputimage);

    SDL_BlitSurface(outputimage, NULL, surface, NULL);

    SDL_FreeSurface(outputimage);


    //IMG_SavePNG(outputImage, "output_image.png");

    // Clean up
    for (int i = 0; i < kernelSize; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

//dilation and erosion to enhance the features


//Dilation = adds pixels to the boundaries of objects in an image.
//we will be using it to Strengthen the grid lines and
//fill in any breaks or gaps in the grid lines.

int White (Uint32 pixel, SDL_PixelFormat* format)
{
    Uint8 r, g, b;
    SDL_GetRGB(pixel, format, &r, &g, &b);
    if (r == 255 && g == 255 && b == 255)
	    return 1;
    return 0;
}

void dilation(SDL_Surface* image)
{
	SDL_Surface* outputimage = SDL_CreateRGBSurface(0, image->w,
                image->h, 32, 0, 0, 0, 0);

	int centerkernel = 1;
	int wimage = image->w;
    	int himage = image->h;
	//Dilate should turn on any pixel that is touching a pixel in the north,
        //east, south, or west direction (no diagonals) that is already turned
        //on in the input.

	SDL_LockSurface(image);
	SDL_LockSurface(outputimage);

	for (int y = centerkernel; y < himage - centerkernel; y++)
	{
		for(int x = centerkernel; x < wimage - centerkernel; x++)
		{
			//the presence of a single foreground pixel anywhere in
                        //the neighborhood will result in a foreground output
			//
                        Uint32 pixeli = ((Uint32*)image->pixels)[y * wimage +x];
			if (White(pixeli, image->format) == 1)
			{
				//the surroundings should be set to white;
				//ys = y surroundings

				for (int ys = -centerkernel;
                                        ys < centerkernel+1; ys++)
				{
					for (int xs  = -centerkernel;
                                                xs< centerkernel+1;xs++)
					{
						((Uint32*)outputimage->pixels)
                                                    [(y + ys) * outputimage->w +
                                                    (x + xs)] = SDL_MapRGB(image
                                                            ->format, 255, 255,
                                                            255);
					}
				}
			}
		}
	}
	//SDL_BlitSurface(outputimage, NULL, image, NULL);

	memcpy(image->pixels, outputimage->pixels, image->w * image->h *
                sizeof(Uint32));
	SDL_UnlockSurface(image);
    	SDL_UnlockSurface(outputimage);
    	SDL_FreeSurface(outputimage);
}




//Erosion removes pixels on object boundaries.
//we will  be using it to removing any small noise or speckles present in the
//image.
// and ensure that numbers in the cells are not connected to the grid lines.


void erosion(SDL_Surface* image)
{
	int centerkernel = 1;
	int wimage = image->w;
    	int himage = image->h;

	SDL_Surface* outputimage = SDL_CreateRGBSurface(0, image->w, image->h,
                32, 0, 0, 0, 0);

	for (int y = centerkernel; y < himage - centerkernel; y++)
	{
		for(int x = centerkernel; x < wimage - centerkernel; x++)
		{

			//a pixel will be set to the background value if any
                        //other pixels in the neighborhood are background.
			int black =  0;

			for (int ys = -centerkernel; ys < centerkernel+1; ys++)
				{
					if (black == 0){
					for (int xs  = -centerkernel; xs<
                                                centerkernel+1;xs++)
					{
						if (black ==0)
						{
							Uint32 pixeli =
                                                            ((Uint32*)image->
                                                             pixels)[(y+ys)
                                                            * wimage +(xs+x)];

							 Uint8 r, g, b;
							 SDL_GetRGB(pixeli,
                                                                 image->format,
                                                                 &r, &g, &b);
							 if (r==0&& g ==
                                                                 0&&b ==0)
								 black =1;


						}
					}
					}
				}
			if (black == 0)
				((Uint32*)outputimage->pixels)
                                    [ y * outputimage->w +x] =
                                    SDL_MapRGB(image->format, 255, 255, 255);

		}
	}
	memcpy(image->pixels, outputimage->pixels, image->w * image->h
                * sizeof(Uint32));
	SDL_UnlockSurface(image);
    	SDL_UnlockSurface(outputimage);
    	SDL_FreeSurface(outputimage);
}

//canny edge detection


double* sobelFilter(SDL_Surface *image, double Gx[3][3], double Gy[3][3],
        double* ang)
{
	int imagew = image->w;
	int imageh = image->h;

	SDL_LockSurface(image);

	Uint32* pixels = (Uint32*) image->pixels;
	double *gradient = malloc(imagew * imageh * sizeof(double));

	for(int y = 1; y < imageh-1; y++)
	{
		for(int x =1; x < imagew -1; x++)
		{
			double gx = 0.0, gy = 0.0;
			for(int ky = -1; ky <= 1; ky++)
			{
				for(int kx = -1; kx <= 1; kx++)
				{
					Uint32 neighborPixel = pixels[(y + ky)
                                            * imagew + x + kx];
					Uint8 intensity;
					SDL_GetRGB(neighborPixel, image->format,
                                                &intensity, &intensity,
                                                &intensity);
					// Get the intensity (0 for black,
                                        // 255 for white)

					gx += Gx[ky + 1][kx + 1] * intensity;
					gy += Gy[ky + 1][kx + 1] * intensity;
				}
			}
			int outputindex = y * imagew + x;

			double angle = atan2f(gy, gx);
			ang[outputindex] = angle;

			float magnitude = sqrt(gx*gx + gy*gy);

			gradient[outputindex] = magnitude;
                        // Store the magnitude
		}
	}

	SDL_UnlockSurface(image);
	return gradient;

}

//Non-Maximal Suppression -> "thin" the edges.
//For each pixel in the gradient image, we compare its magnitude to the
//magnitude of its neighbors along the gradient direction.
//If the pixel's magnitude  > its neighbors', it remains unchanged;
//otherwise, it's set to zero.
double* nonMaximalSuppressionAndHysteresis(double* gradient, double*
        ang, int width, int height, double lowThreshold, double highThreshold)
{
    double* edge = (double*)calloc(width * height, sizeof(double));
    if (!edge)
        return NULL;

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
	{
            int index = y * width + x;
            double angle = ang[index];


	    // 1st -> find neighboring pixels based on the gradient direction
            int dx1, dy1, dx2, dy2;
	    //MI_PI_4 is 45
            if ((angle >= -M_PI_4 && angle < M_PI_4) ||
                    (angle <= -3 * M_PI_4 || angle >= 3 * M_PI_4)) {
                dx1 = 1; dy1 = 0; dx2 = -1; dy2 = 0;
            } else if ((angle >= M_PI_4 && angle < 3 * M_PI_4) ||
                    (angle <= -M_PI_4 && angle >= -3 * M_PI_4)) {
                dx1 = 0; dy1 = 1; dx2 = 0; dy2 = -1;
            } else if (angle >= 0) {
                dx1 = 1; dy1 = 1; dx2 = -1; dy2 = -1;
            } else {
                dx1 = 1; dy1 = -1; dx2 = -1; dy2 = 1;
            }


            // Non-maximal suppression

	    //It works by iterating through all pixel values, comparing the
            //current value with the pixel value in the positive and negative
            //gradient
	    //directions, and suppressing the current value if it does not
            //have the highest magnitude relative to its neighbors


            if (gradient[index] >= gradient[(y + dy1) * width + x + dx1] &&
                gradient[index] >= gradient[(y + dy2) * width + x + dx2]) {

                // Hysteresis thresholding

                if (gradient[index] > highThreshold) //strong edge
		{
                    edge[index] = 255;
                }
		else if (gradient[index] > lowThreshold)
                    //maybe a edge(potentiel)
		{
                    edge[index] = 127;
                }
            }
        }
    }

    // Edge tracing

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
	{
            int index = y * width + x;

            if (edge[index] == 127)
	    {
                // Check if any strong edge neighbors exist
                for (int dy = -1; dy <= 1; dy++)
		{
                    for (int dx = -1; dx <= 1; dx++)
		    {
                        if (edge[(y + dy) * width + x + dx] == 255)
                            //if one exits than become a strong
			{
                            edge[index] = 255;
                            break;
                        }
                    }
                }
                // If no strong edge neighbors, SUPPRESS
                if (edge[index] != 255)
                    edge[index] = 0;
            }
        }
    }

    return edge;
}









void Canny_edge_result (SDL_Surface* image)
{
	int width = image->w;
	int height = image->h;
	double Gx[3][3] = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};

	double Gy[3][3] = {
		{-1, -2, -1},
		{ 0,  0,  0},
		{ 1,  2,  1}
	};


	double *ang = calloc(image->w * image->h,sizeof(double));


	double* gradient = sobelFilter(image, Gx, Gy,ang) ;

	// Determine the maximum gradient value for normalization
	double maxGradient = 0;
	for (int i = 0; i < width * height; i++) {
		if (gradient[i] > maxGradient) {
			maxGradient = gradient[i];
		}
	}

	//visualizeGradient(image, gradient);
	//IMG_SaveJPG(image, "grad.jpg", 100);


	double* edge = nonMaximalSuppressionAndHysteresis(
                gradient,ang, image->w, image->h, maxGradient *
                0.35, maxGradient*0.7);

	SDL_LockSurface(image);

	Uint32* pixels = (Uint32*)image->pixels;


	for (int y = 0; y < height; y++)
	{
		for (int x =0; x < width; x++)
		{
			int index = y * width + x;


			if (edge[index] == 0)
			{

				pixels[index] =
                                    SDL_MapRGB(image->format, 0, 0, 0);
			} else
			{

				pixels[index] =
                                    SDL_MapRGB(image->format, 255, 255, 255);
			}
		}
	}


	SDL_UnlockSurface(image);
	free(ang);
	free(gradient);
	free(edge);
}


