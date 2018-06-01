#ifndef STORAGE_H
#define STORAGE_H

#include "../library/clipboard.h"
#include "../utils/list.h"

#include <pthread.h>

#define MAX_ELEMENTS 10

/**
 * @brief Clipboard storage element.
 * @see storage
 */
typedef struct element {
    char *buf;
    size_t len;
    unsigned long timestamp;
    unsigned long hash;
} element_t;

/**
 * @brief Clipboard Storage containing MAX_ELEMENTS.
 * @see element
 */
typedef struct storage {
    element_t **elements;
} storage_t;

/**
 * @brief Each mutex is responsible for one region of the clipboard.
 */
pthread_mutex_t m[MAX_ELEMENTS];
/**
 * @brief Each cond is responsible for one region of the clipboard.
 */
pthread_cond_t c[MAX_ELEMENTS];

/**
 * @brief Mutex for accessing the remote connections list.
 */
pthread_mutex_t remote_connections_mutex;
/**
 * @brief Mutex for accessing the local connections list.
 */
pthread_mutex_t local_connections_mutex;

/**
 * @brief List storage for remote clients.
 */
list(int, remote_connections);

/**
 * @brief List storage for local clients.
 */
list(int, local_connections);

/**
 * @brief Default clipboard message store
 */
storage_t *msg_store;

/**
 * @brief Create new cliboard storage.
 *
 * @return New storage
 */
storage_t *new_storage(void);

/**
 * @brief Put message into storage.
 *
 * @param int Region to place message
 * @param long message timestamp
 * @param long message hash
 * @param size_t message size
 * @param char* message buffer
 *
 * @return -1 fail, 0 success
 */
int put_message(int, unsigned long, unsigned long, size_t, char *);

/**
 * @brief Get message from storage.
 *
 * @param int Region
 *
 * @return Element from region in storage
 */
element_t *get_message(int);

/**
 * @brief Dump storage to stdout.
 */
void print_storage(void);

/**
 * @brief Free storage structure.
 *
 * @param storage_t* to free
 */
void free_storage(storage_t *);

#endif
