#ifndef STORAGE_H
#define STORAGE_H

#include "../utils/list.h"
#include "../utils/log.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../library/clipboard.h"

#define MAX_ELEMENTS 10
#define MAX_MESSAGE_SIZE 4096

typedef struct element {
    char *buf;
    size_t len;
    unsigned long timestamp;
    int hash;
} element_t;

typedef struct storage {
    element_t **elements;
} storage_t;

typedef struct r_socket {
    int sd;
} r_socket_t;

pthread_mutex_t m[MAX_ELEMENTS];
pthread_cond_t c[MAX_ELEMENTS];

pthread_mutex_t remote_connections_mutex;
pthread_mutex_t local_connections_mutex;

list(int, remote_connections);
list(int, local_connections);

storage_t *msg_store;
storage_t *new_storage();
int put_message(int region, unsigned long timestamp, int hash, int len,
                char *buf);
element_t *get_message(int region);
void print_storage();
void free_storage(storage_t *to_free);

#endif
