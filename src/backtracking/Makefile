# Makefile

CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -fsanitize=address -g
LDFLAGS =-fsanitize=address -lm
LDLIBS =

SRC = allocator.c main.c solver.c filestream.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=.d}

solver: ${OBJ}
-include ${DEP}
.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}
	${RM} solver

# END
