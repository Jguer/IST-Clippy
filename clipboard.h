#ifndef CLIPBOARD_HEADER
#define CLIPBOARD_HEADER

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STRING_SIZE 256

typedef struct element_t {
    char string[256];
} element;

typedef struct clipboard_t {
    element head[10];
    uint_fast32_t count;
} clipboard;

int clipboard_connect(char *clipboard_dir);
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count);
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count);
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count);
void clipboard_close(int clipboard_id);

#endif
