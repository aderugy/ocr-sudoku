# Makefile

CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -fsanitize=address -Wall -O3
LDFLAGS = -fsanitize=address 
LDLIBS = `pkg-config --libs gtk+-3.0`

EXE = test

all: $(EXE)

$(foreach f, $(EXE), $(eval $(f):))

.PHONY: clean

clean:
	${RM} $(EXE)

# END
