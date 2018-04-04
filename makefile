CC = gcc
CFLAGS = -march=native -std=gnu11 -Wall -Wextra -Werror -g -Wpedantic -Wshadow -Wstrict-overflow -fno-strict-aliasing
CFLAGS_RELEASE = -march=native -std=gnu11 -O2 -Os
BINARY = clippy

all: main

default: main

main: main.c clipboard.c
	$(CC) main.c clipboard.c $(CFLAGS) -o $(BINARY)

clean:
	-@rm bin/$(BINARY)*

.PHONY: all clean test default
