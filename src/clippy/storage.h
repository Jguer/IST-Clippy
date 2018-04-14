#ifndef STORAGE_HEADER_GUARD
#define STORAGE_HEADER_GUARD
#include "../utils/log.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct element {
    char *buf;
    size_t len;
    unsigned long timestamp;
} element_t;

typedef struct storage {
    element_t **elements;
} storage_t;

storage_t *msg_store;

storage_t *new_storage();
int put_message(int region, long int timestamp, char *buf, int len);
element_t *get_message(int region);

#endif
