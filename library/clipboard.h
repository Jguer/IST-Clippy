#ifndef CLIPBOARD_HEADER
#define CLIPBOARD_HEADER

#include "../utils/log.h"

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define MESSAGE_SIZE 4096

enum op { COPY, PASTE, WAIT };

#pragma pack(1) // this helps to avoid serialization while sending over network.
typedef struct header {
    enum op op;
    int region;
    size_t data_size;
    unsigned long timestamp;
    int hash;
} header_t;
#pragma pack(0) // turn packing off

int clipboard_connect(char *);
int clipboard_copy(int, int, void *, size_t);
int clipboard_paste(int, int, void *, size_t);
int clipboard_wait(int, int, void *, size_t);
void clipboard_close(int);
int ht_hash(char *, int);

#endif
