#pragma once

#include <stdio.h>

#include <stdlib.h>
#include <png.h>
#include <dirent.h>

void get_png_files(const char *folder, char ***fileNames, int *numFiles) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(folder);

    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    // Count the number of .png files in the directory
    *numFiles = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".png") != NULL) {
            (*numFiles)++;
        }
    }

    // Allocate memory for file names
    *fileNames = (char **)malloc(sizeof(char *) * (*numFiles));

    // Reset directory stream to the beginning
    rewinddir(dir);

    // Store the names of .png files
    int fileIndex = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".png") != NULL) {
            (*fileNames)[fileIndex] = strdup(entry->d_name);
            fileIndex++;
        }
    }

    closedir(dir);
}

void free_file_names(char ***fileNames, int numFiles) {
    for (int j = 0; j < numFiles; j++) {
        free((*fileNames)[j]);
    }

    free(*fileNames);
}

void read_png_file(const char *filename, double **binaryMap, int *width, int *height) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) abort();

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);
    png_read_info(png, info);

    *width = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    *binaryMap = (double *)malloc(sizeof(double *) * (*height) * (*width));

    png_bytep row_pointers[*height];
    for (int y = 0; y < *height; y++) {
        row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    for (int y = 0; y < *height; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < *width; x++) {
            png_bytep px = &(row[x * 4]);  // Assuming RGBA format

            // Assuming RGBA format, adjust accordingly if your image uses a different format
            if (px[3] == 0) {
                (*binaryMap)[y * 28 + x] = 0;  // Transparent pixel
            } else {
                // Calculate grayscale value from RGB
                double grayscale = (0.299 * px[0] + 0.587 * px[1] + 0.114 * px[2]) / 255.0;

                // Set pixel value based on grayscale value
                (*binaryMap)[y * 28 + x] = (grayscale > 0.5) ? 0 : 1;
            }

        }
    }

    for (int y = 0; y < *height; y++) {
        free(row_pointers[y]);
    }

    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
}

void load_images(double ****images, char **png_files, int num_files) {
    *images = malloc(9 * sizeof(double ***));
    int width;
    int height;
    char *path;

    for (size_t i = 0; i < 9; i++)
    {
        (*images)[i] = malloc(num_files * sizeof(double **));

        for (int j = 0; j < num_files; j++)
        {
            asprintf(&path, "./data/digits_im/%zu/%s", i, png_files[j]);
            read_png_file(path, (*images)[i] + j, &width, &height);
            free(path);
        }
    }
}

void free_images(double ****images, int num_files) {
    for (size_t i = 0; i < 9; i++)
    {
        for (int j = 0; j < num_files; j++) {
            free((*images)[i][j]);
        }
        free((*images)[i]);
    }

    free(*images);
}
