#pragma once
#ifndef STORAGE_HEADER_GUARD
#define STORAGE_HEADER_GUARD

#include "utils/list.h"
#include "utils/log.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ELEMENTS 10

typedef struct element {
    char *buf;
    size_t len;
    unsigned long timestamp;
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

list(int, remote_connections);

storage_t *msg_store;
storage_t *new_storage();
int put_message(int region, long int timestamp, char *buf, int len);
element_t *get_message(int region);

#endif
