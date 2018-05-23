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
    int data_size;
    unsigned long timestamp;
    int hash;
} header_t;
#pragma pack(0) // turn packing off

int clipboard_connect(char *clipboard_dir);
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count);
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count);
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count);
void clipboard_close(int clipboard_id);
int ht_hash(char *s);

#endif
