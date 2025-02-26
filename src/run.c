#include <gtk/gtk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <emmintrin.h>
#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// include all main here
#include "./ocr/helper.h"
#include "./ocr/network.h"
#include "./ocr/ocr.h"
#include "./ocr/readSDL.h"
#include "./ocr/thread_pool.h"
// training

#include "./backtracking/allocator.h"
#include "./backtracking/filestream.h"
#include "./backtracking/solver.h"
#include "./backtracking/usage.h"
// main

#include "./image_processing/image.h"
#include "./image_processing/processing.h"
// square_detection

#include "./image_processing/hough.h"
#include "./image_processing/detection.h"

#include "./ui.h"
// detection

int original_image_width = 0;
int original_image_height = 0;




int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.sudocul.MyApp", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
  }
