# Makefile

CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -fsanitize=address -lm -g `pkg-config --cflags sdl2 SDL2_image`
LDFLAGS = -fsanitize=address -lm
LDLIBS =`pkg-config --libs sdl2 SDL2_image`

SRC = helper.c network.c ocr.c training.c readSDL.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=.d}

training: ${OBJ}

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}
	${RM} training

# END
