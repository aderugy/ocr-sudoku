#pragma once
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cairo.h>
#include <unistd.h>
#include "process.h"

#define _GNU_SOURCE

GtkWidget *grayscale_button;
GtkWidget *contrast_button;
GtkWidget *reducenoise_button;
GtkWidget *threshold_button;
GtkWidget *erosion_button;
GtkWidget *dialation_button;
GtkWidget *canny_button;
GtkWidget *hough_button;
GtkWidget *hough_average_button;
GtkWidget *drawsquare_button;
GtkWidget *extracted_button;
GtkWidget *get_grid_button;
GtkWidget *solve_grid_button;
GtkWidget *sudokuGrid;
GtkWidget *rotate_button;

static GtkWidget *image;
int is_loaded = 0;
char *file_path;
static GtkWidget *sol_image;

int in_solve_window = 0;
int in_main_window = 1;
int in_steps_window = 0;

static void get_the_grid(GtkWidget *widget);
static void solve_image(GtkWidget *widget);




int start_matrix[9][9];


int end_matrix[9][9];

void initialize_start_matrix() {
	// Initialize start_matrix with your desired values
	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			start_matrix[row][col] = col + 1;
		}
	}
}

void initialize_end_matrix() {
	// Initialize end_matrix with your desired values
	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			end_matrix[row][col] = 9 - col;
		}
	}
}


static void on_steps_button_clicked(GtkButton *button, gpointer steps_window) {
	gtk_widget_hide(GTK_WIDGET(
				gtk_widget_get_toplevel(GTK_WIDGET(button))));
	gtk_widget_show_all(GTK_WIDGET(steps_window));
	in_steps_window = 1;
	in_main_window = 0;
	initialize_start_matrix();
	initialize_end_matrix();
}


static void on_back_button_clicked(GtkButton *button, gpointer menu_window) {
	gtk_widget_hide(GTK_WIDGET(
				gtk_widget_get_toplevel(GTK_WIDGET(button))));
	gtk_widget_show_all(GTK_WIDGET(menu_window));
	in_main_window = 1;
	in_steps_window = 0;
	initialize_start_matrix();
	initialize_end_matrix();
}


static void on_solve_button_clicked(GtkButton *button, gpointer solve_window) {
	gtk_widget_hide(GTK_WIDGET(
				gtk_widget_get_toplevel(GTK_WIDGET(button))));
	gtk_widget_show_all(GTK_WIDGET(solve_window));
	in_solve_window = 1;
	in_main_window = 0;
	initialize_start_matrix();
	initialize_end_matrix();
}


static void on_solve_back_button_clicked(GtkButton *button,
		gpointer menu_window) {
	gtk_widget_hide(GTK_WIDGET(
				gtk_widget_get_toplevel(GTK_WIDGET(button))));
	gtk_widget_show_all(GTK_WIDGET(menu_window));
	in_main_window = 1;
	in_solve_window = 0;
	initialize_start_matrix();
	initialize_end_matrix();
}


void update_matrices_from_grid(const char *grid_string, int var) {
	int row = 0, col = 0;
	while (*grid_string) {
		if (*grid_string >= '1' && *grid_string <= '9') {
			if (var == 0){
				start_matrix[row][col] = *grid_string - '0';
			}
			else {
				end_matrix[row][col] = *grid_string - '0';
			}
		} else if (*grid_string == '.') {
			start_matrix[row][col] = 0;
		}

		// Move to next cell
		col++;
		if (col == 9) {
			col = 0;
			row++;
		}

		// Skip to next character
		grid_string++;

		// Stop if we reach the end of the grid
		if (row == 9) {
			break;
		}
	}
}


static void get_the_grid(GtkWidget *widget) {
	GtkWidget *img;
	if(in_solve_window){
		img = sol_image;
	}
	else{
		img = image;
	}

	if (!is_loaded) {
		g_print("Image has not been loaded\n");
		return;
	}

	// Open the file containing the grid
	FILE *file = fopen("./../build/grids/grid_01", "r");
	if (file == NULL) {
		g_print("Failed to open grid file\n");
		return;
	}

	char grid_string[82];
	char line[20];        // Temporary buffer for each line
	int index = 0;

	// Read the file line by line
	while (fgets(line, sizeof(line), file)) {
		for (int i = 0; line[i] != '\0' && index < 81; i++) {
			if ((line[i] >= '0' && line[i] <= '9') ||
					line[i] == '.') {
				grid_string[index++] = line[i];
			}
		}
	}
	grid_string[index] = '\0'; // Null-terminate the string
	fclose(file);

	// Update start_matrix from grid_string
	update_matrices_from_grid(grid_string, 0);

	// Create a new grid for the Sudoku puzzle
	sudokuGrid = gtk_grid_new();

	// Populate the grid with buttons
	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			char buffer[3];
			if (start_matrix[row][col] == 0) {
				snprintf(buffer, sizeof(buffer), "%s", "");
			} else {
	snprintf(buffer, sizeof(buffer), "%d", start_matrix[row][col]);
			}

	GtkWidget *number_button = gtk_button_new_with_label(buffer);
			gtk_grid_attach(GTK_GRID(sudokuGrid),
					number_button, col, row, 1, 1);
		}
	}

	// Get parent container of sudoku_image
	GtkWidget *parent_container = gtk_widget_get_parent(img);

	// Remove sudoku_image from its parent container
	gtk_container_remove(GTK_CONTAINER(parent_container), img);

	// Attach the new Sudoku grid to the parent container
	gtk_container_add(GTK_CONTAINER(parent_container), sudokuGrid);

	// Make sure the grid and its children are visible
	gtk_widget_show_all(parent_container);
}

static void solve_image(GtkWidget *widget) {
	FILE *file = fopen("./../build/grids/grid_01.result", "r");
	if (file == NULL) {
		g_print("Failed to open grid file\n");
		return;
	}

char grid_string[82]; // 81 characters for the grid + 1 for null terminator
	char line[20];        // Temporary buffer for each line
	int index = 0;

	// Read the file line by line
	while (fgets(line, sizeof(line), file)) {
		// Process each character in the line
		for (int i = 0; line[i] != '\0' && index < 81; i++) {
		if ((line[i] >= '0' && line[i] <= '9') || line[i] == '.') {
				grid_string[index++] = line[i];
			}
		}
	}
	grid_string[index] = '\0'; // Null-terminate the string
	fclose(file);

	// Call update_matrices_from_grid function
	update_matrices_from_grid(grid_string, 1);


	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
	GtkWidget *child = gtk_grid_get_child_at(GTK_GRID(sudokuGrid),
					col, row);

			if (GTK_IS_BUTTON(child)) {
				char buffer[3];
		snprintf(buffer, sizeof(buffer), "%d", end_matrix[row][col]);
			gtk_button_set_label(GTK_BUTTON(child), buffer);

		// Check if the number has changed and change color if it has
		if (start_matrix[row][col] == 0 && end_matrix[row][col] != 0) {
			// Change button color to indicate it's a new number
				GtkStyleContext *context =
					gtk_widget_get_style_context(child);
					gtk_style_context_add_class(context,
							"changed-number");
				}
			}
		}
	}

	gtk_widget_show_all(sudokuGrid);
}



static void on_file_chosen(GtkFileChooserButton *button, gpointer user_data) {
	gchar *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(button));

	//    image_solve();
	if (uri) {
		char *file_path = g_filename_from_uri(uri, NULL, NULL);
		g_free(uri);

		process_image(file_path);
		process_autorot(file_path);
		image_solve();

		GdkPixbuf *pixbuf =gdk_pixbuf_new_from_file_at_scale(file_path,
				600, 600, TRUE, NULL);
		if (pixbuf) {
			gtk_image_set_from_pixbuf(GTK_IMAGE(user_data),
					pixbuf);
			g_object_unref(pixbuf);
			is_loaded = 1;
		}

		g_free(file_path);
	}
}


static void update_button_style(GtkWidget *button, gboolean is_active) {
	GtkStyleContext *context = gtk_widget_get_style_context(button);
	if (is_active) {
		gtk_style_context_add_class(context, "button-active");
		gtk_style_context_remove_class(context, "button-inactive");
	} else {
		gtk_style_context_remove_class(context, "button-active");
		gtk_style_context_add_class(context, "button-inactive");
	}
}

static void on_filter_button_clicked(GtkWidget *widget, gpointer data) {
	if (!is_loaded) {
		g_print("Image has not been loaded\n");
		return;
	}

	char *image_path = (char *)data;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(image_path, 600,
			600, TRUE, NULL);
	if (pixbuf) {
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
		g_object_unref(pixbuf);
		g_print("Image filter applied from: %s\n", image_path);
	} else {
		g_print("Failed to load image from: %s\n", image_path);
	}

	// Update the states of all filter buttons
	gtk_widget_set_sensitive(grayscale_button, widget != grayscale_button);
	gtk_widget_set_sensitive(contrast_button, widget != contrast_button);

	gtk_widget_set_sensitive(reducenoise_button, widget !=
			reducenoise_button);
	gtk_widget_set_sensitive(threshold_button, widget != threshold_button);

	gtk_widget_set_sensitive(erosion_button, widget != erosion_button);
	gtk_widget_set_sensitive(dialation_button, widget != dialation_button);

	gtk_widget_set_sensitive(canny_button, widget != canny_button);


	gtk_widget_set_sensitive(hough_button, widget != hough_button);
	gtk_widget_set_sensitive(hough_average_button, widget !=
			hough_average_button);
	gtk_widget_set_sensitive(drawsquare_button, widget !=
			drawsquare_button);
	gtk_widget_set_sensitive(extracted_button, widget !=
			extracted_button);



	update_button_style(grayscale_button, widget == grayscale_button);
	update_button_style(contrast_button, widget == contrast_button);

	update_button_style(reducenoise_button, widget == reducenoise_button);
	update_button_style(threshold_button, widget == threshold_button);

	update_button_style(erosion_button, widget == erosion_button);
	update_button_style(dialation_button, widget == dialation_button);

	update_button_style(canny_button, widget == canny_button);



	update_button_style(hough_button, widget == hough_button);
	update_button_style(hough_average_button, widget ==
			hough_average_button);
	update_button_style(drawsquare_button, widget == drawsquare_button);
	update_button_style(extracted_button, widget == extracted_button);
}
///





static void on_filter_button_clicked2(GtkWidget *widget, gpointer data) {
	if (!is_loaded) {
		g_print("Image has not been loaded\n");
		return;
	}

	// process_autorot();

	char *image_path = (char *)data;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(image_path,
			600,600,TRUE,NULL);
	if (pixbuf) {
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
		g_object_unref(pixbuf);
		g_print("Image filter applied from: %s\n", image_path);
	} else {
		g_print("Failed to load image from: %s\n", image_path);
	}

}






///
cairo_surface_t* create_sudoku_surface() {
	const int cell_size = 50; // Size of each cell in the grid
	const int grid_size = 9 * cell_size;
	const int thick_line_width = 4; // Line width for the thicker lines
	const int thin_line_width = 1;  // Line width for the thinner lines

	// Create a Cairo surface to draw on
	cairo_surface_t *surface =
		cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
				grid_size + thick_line_width, grid_size +
				thick_line_width);
	cairo_t *cr = cairo_create(surface);

	// Set the background to white
	cairo_set_source_rgb(cr, 1, 1, 1); // White
	cairo_paint(cr);

	// Set the source color to black for the grid
	cairo_set_source_rgb(cr, 0, 0, 0); // Black

	// Draw the grid lines
	for (int i = 0; i <= 9; ++i) {
		// Set line width
		cairo_set_line_width(cr, (i % 3 == 0) ? thick_line_width :
				thin_line_width);

		// Horizontal lines
		int y = i * cell_size + (i >= 3) + (i >= 6);
		cairo_move_to(cr, 0, y);
		cairo_line_to(cr, grid_size, y);

		// Vertical lines
		int x = i * cell_size + (i >= 3) + (i >= 6);
		cairo_move_to(cr, x, 0);
		cairo_line_to(cr, x, grid_size);

		cairo_stroke(cr);
	}

	// Draw the numbers
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 20);

	for (int row = 0; row < 9; ++row) {
		for (int col = 0; col < 9; ++col) {
			int num = end_matrix[row][col];
			if (num != 0) {
				char num_text[2];
				snprintf(num_text,
						sizeof(num_text), "%d", num);

				cairo_text_extents_t extents;
				cairo_text_extents(cr, num_text, &extents);
				double text_x = col * cell_size +
					(cell_size - extents.width)
					/ 2 - extents.x_bearing;
				double text_y = row * cell_size +
					(cell_size - extents.height)
					/ 2 - extents.y_bearing;

				cairo_move_to(cr, text_x, text_y);
				cairo_show_text(cr, num_text);
			}
		}
	}

	cairo_destroy(cr);
	return surface;
}



static gboolean display_next_image(gpointer user_data) {
	static int current_image = 0;
	const char *image_paths[] = {
		"./../build/images/grayscale.jpg",
		"./../build/images/contrast.jpg",
		"./../build/images/reducenoise.jpg",
		"./../build/images/inverse.jpg",
		"./../build/images/erosion.jpg",
		"./../build/images/dilation.jpg",
		"./../build/images/canny.jpg",
		"./../build/images/hough.jpg",
		"./../build/images/hough_average.jpg",
		"./../build/images/drawsquares.jpg",
		"./../build/images/extracted.jpg"
	};
	const int total_images = sizeof(image_paths) / sizeof(image_paths[0]);
	const int grid_display_stage = total_images;
	const int solve_stage = total_images + 1;

	if (current_image < total_images) {
		// Display each image
		GdkPixbuf *pixbuf =
			gdk_pixbuf_new_from_file_at_scale(
					image_paths[current_image],
					600, 600, TRUE, NULL);
		if (pixbuf) {
			gtk_image_set_from_pixbuf(GTK_IMAGE(sol_image),
					pixbuf);
			g_object_unref(pixbuf);
		}
		current_image++;
	}else if (current_image == grid_display_stage) {
		// Display the grid
		gtk_widget_hide(sol_image); // Hide or remove the image widget
		image_solve(); // Get the grid from the 81 images
		get_the_grid(NULL);
		current_image++;
	} else if (current_image == solve_stage) {
		// Solve the grid
		solve_image(NULL);
		current_image++;
		return FALSE; // Return FALSE to stop the timeout
	}

	// Continue the timeout as long as we haven't reached the final stage
	return current_image <= solve_image;
}

static void on_epic_button_clicked(GtkWidget *widget) {
	if (!is_loaded){
		g_print("Image has not been loaded\n");
		return;
	}

	g_timeout_add_seconds(1, display_next_image, NULL);
	//image_solve();
}




static void save_sudoku_as_png(GtkWidget *widget, gpointer user_data) {
	const char* filename = g_object_get_data(G_OBJECT(widget), "filename");
	if (filename) {
		// Call the function with the filename

		cairo_surface_t* surface = create_sudoku_surface();

		// Save the surface to a PNG file
		cairo_surface_write_to_png(surface, filename);

		// Clean up
		cairo_surface_destroy(surface);
	}
}


static void save_sudoku_as_jpeg(GtkWidget *widget, gpointer user_data) {
	const char* filename = g_object_get_data(G_OBJECT(widget), "filename");
	if (filename) {
		cairo_surface_t* surface = create_sudoku_surface();

		// Convert the Cairo surface to a GdkPixbuf
		GdkPixbuf *pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0,
				cairo_image_surface_get_width(surface),
				cairo_image_surface_get_height(surface));

		// Save the GdkPixbuf as a JPEG file
	gdk_pixbuf_save(pixbuf, filename, "jpeg", NULL, "quality", "90", NULL);

		// Clean up
		g_object_unref(pixbuf);
		cairo_surface_destroy(surface);
	}
}


void activate(GtkApplication* app, gpointer user_data) {
	GtkBuilder *builder1, *builder2, *solve_builder;
	GtkWidget *window1, *window2, *solve_window;
	GtkWidget *steps_button, *back_preprocess_button;
	GtkWidget *s_button, *back_s_button;

	GtkCssProvider *css_provider = gtk_css_provider_new();
	gtk_css_provider_load_from_path(css_provider, "style.css", NULL);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
			GTK_STYLE_PROVIDER(css_provider),
			GTK_STYLE_PROVIDER_PRIORITY_USER);


	// Load the first Glade file
	builder1 = gtk_builder_new_from_file("gladetest.glade");
	window1 = GTK_WIDGET(gtk_builder_get_object(builder1, "windowtest"));
	if (window1 != NULL && GTK_IS_WINDOW(window1)) {
		gtk_application_add_window(app, GTK_WINDOW(window1));
	}

	builder2 = gtk_builder_new_from_file("secondtest.glade");
	window2 = GTK_WIDGET(gtk_builder_get_object(builder2,
				"window_preprocess"));

	solve_builder = gtk_builder_new_from_file("solve.glade");
	solve_window = GTK_WIDGET(gtk_builder_get_object(solve_builder,
				"window_solve"));

	steps_button = GTK_WIDGET(gtk_builder_get_object(builder1,
				"steps_button"));

	if (steps_button != NULL && GTK_IS_BUTTON(steps_button)) {
		g_signal_connect(steps_button, "clicked",
				G_CALLBACK(on_steps_button_clicked), window2);
	}

	back_preprocess_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"back_button"));

	if (back_preprocess_button != NULL &&
			GTK_IS_BUTTON(back_preprocess_button)) {
		g_signal_connect(back_preprocess_button, "clicked",
				G_CALLBACK(on_back_button_clicked), window1);
	}

	s_button = GTK_WIDGET(gtk_builder_get_object(builder1,
				"solve_button"));

	if (s_button != NULL && GTK_IS_BUTTON(s_button)) {
		g_signal_connect(s_button, "clicked",
				G_CALLBACK(on_solve_button_clicked),
				solve_window);
	}

	back_s_button = GTK_WIDGET(gtk_builder_get_object(solve_builder,
				"back_button"));

	if (back_s_button != NULL && GTK_IS_BUTTON(back_s_button)) {
		g_signal_connect(back_s_button, "clicked",
				G_CALLBACK(on_solve_back_button_clicked),
				window1);
	}


	image = GTK_WIDGET(gtk_builder_get_object(builder2, "sudoku_image"));

	// CALL
	GtkWidget *file_chooser_button =
		GTK_WIDGET(gtk_builder_get_object(builder2,
					"file_chooser_button"));
	g_signal_connect(file_chooser_button, "file-set",
			G_CALLBACK(on_file_chosen), image);

	sol_image = GTK_WIDGET(gtk_builder_get_object(solve_builder,
				"sudoku_image"));

	GtkWidget *solve_file_chooser_button =
		GTK_WIDGET(gtk_builder_get_object(solve_builder,
					"file_chooser_button"));
	g_signal_connect(solve_file_chooser_button, "file-set",
			G_CALLBACK(on_file_chosen), sol_image);

	// EPIC BUTTON
	GtkWidget *epic_button =
		GTK_WIDGET(gtk_builder_get_object(solve_builder,
					"solve_button"));
	g_signal_connect(epic_button, "clicked",
			G_CALLBACK(on_epic_button_clicked), NULL);

	// PRE PROCESSING
	grayscale_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"gs_button"));
	g_signal_connect(grayscale_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/grayscale.jpg");

	contrast_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"co_button"));
	g_signal_connect(contrast_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/contrast.jpg");

	reducenoise_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"re_button"));
	g_signal_connect(reducenoise_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/reducenoise.jpg");

	threshold_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"th_button"));
	g_signal_connect(threshold_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/inverse.jpg");

	erosion_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"er_button"));
	g_signal_connect(erosion_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/erosion.jpg");

	dialation_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"di_button"));
	g_signal_connect(dialation_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/dilation.jpg");


	canny_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"ca_button"));
	g_signal_connect(canny_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/canny.jpg");



	// PROCESSING
	hough_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"ho_button"));
	g_signal_connect(hough_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/hough.jpg");


	hough_average_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"av_button"));
	g_signal_connect(hough_average_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/hough_average.jpg");

	drawsquare_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"dr_button"));
	g_signal_connect(drawsquare_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/drawsquares.jpg");

	extracted_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"ss_button"));
	g_signal_connect(extracted_button, "clicked",
			G_CALLBACK(on_filter_button_clicked),
			"./../build/images/extracted.jpg");


	rotate_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"rot_button"));
	g_signal_connect(rotate_button, "clicked",
			G_CALLBACK(on_filter_button_clicked2),
			"./../build/images/autorot.jpg");


	GtkWidget *save_png_button =
		GTK_WIDGET(gtk_builder_get_object(solve_builder,
					"png_button"));
	GtkWidget *save_jpeg_button =
		GTK_WIDGET(gtk_builder_get_object(solve_builder,
					"jpeg_button"));
	GtkWidget *save_jpg_button =
		GTK_WIDGET(gtk_builder_get_object(solve_builder,
					"jpg_button"));

	// Set filenames for each button
	g_object_set_data(G_OBJECT(save_png_button), "filename",
			"png_sudoku_grid.png");
	g_object_set_data(G_OBJECT(save_jpeg_button), "filename",
			"jpeg_sudoku_grid.jpeg");
	g_object_set_data(G_OBJECT(save_jpg_button), "filename",
			"jpg_sudoku_grid.jpg");

	// Connect signals
	g_signal_connect(save_png_button, "clicked",
			G_CALLBACK(save_sudoku_as_png), NULL);
	g_signal_connect(save_jpeg_button, "clicked",
			G_CALLBACK(save_sudoku_as_jpeg), NULL);
	g_signal_connect(save_jpg_button, "clicked",
			G_CALLBACK(save_sudoku_as_jpeg), NULL);


	GtkWidget *png_button =
		GTK_WIDGET(gtk_builder_get_object(builder2, "png_button"));
	GtkWidget *jpeg_button =
		GTK_WIDGET(gtk_builder_get_object(builder2, "jpeg_button"));
	GtkWidget *jpg_button =
		GTK_WIDGET(gtk_builder_get_object(builder2, "jpg_button"));

	// Set filenames for each button
	g_object_set_data(G_OBJECT(png_button),
			"filename", "1sudoku_grid.png");
	g_object_set_data(G_OBJECT(jpeg_button),
			"filename", "2sudoku_grid.jpeg");
	g_object_set_data(G_OBJECT(jpg_button),
			"filename", "3sudoku_grid.jpg");

	// Connect signals
	g_signal_connect(png_button,
			"clicked", G_CALLBACK(save_sudoku_as_png), NULL);
	g_signal_connect(jpeg_button,
			"clicked", G_CALLBACK(save_sudoku_as_jpeg), NULL);
	g_signal_connect(jpg_button,
			"clicked", G_CALLBACK(save_sudoku_as_jpeg), NULL);


	// SOLVE
	get_grid_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"get_grid_button"));
	if (get_grid_button != NULL && GTK_IS_BUTTON(get_grid_button)) {
		g_signal_connect(get_grid_button, "clicked",
				G_CALLBACK(get_the_grid),NULL);
	}

	solve_grid_button = GTK_WIDGET(gtk_builder_get_object(builder2,
				"solve_grid_button"));
	if (solve_grid_button != NULL && GTK_IS_BUTTON(solve_grid_button)) {
		g_signal_connect(solve_grid_button,
				"clicked",
				G_CALLBACK(solve_image), sudokuGrid);
	}


	gtk_widget_show_all(window1);
}
