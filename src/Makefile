CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -fsanitize=address -lm -g `pkg-config --cflags sdl2 SDL2_image gtk+-3.0`
LDFLAGS = -fsanitize=address -lm
LDLIBS = `pkg-config --libs sdl2 SDL2_image gtk+-3.0`

OBJDIR = obj

SOURCES = $(wildcard *.c) \
          $(wildcard backtracking/*.c) \
          $(wildcard image_processing/*.c) \
          $(wildcard ui/*.c) \
          $(wildcard ocr/*.c)

OBJECTS = $(patsubst %.c, $(OBJDIR)/%.o, $(SOURCES))

all: run

$/run: $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	rm run

.PHONY: all clean

