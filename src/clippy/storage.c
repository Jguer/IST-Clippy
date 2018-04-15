#include "storage.h"

#define MAX_ELEMENTS 10

pthread_mutex_t m[MAX_ELEMENTS] = {PTHREAD_MUTEX_INITIALIZER};

storage_t *new_storage() {
    storage_t *nouveau = (storage_t *)malloc(sizeof(storage_t));
    if (nouveau == NULL) {
        log_fatal("Unable to malloc storage_t");
        return NULL;
    }

    nouveau->elements = (element_t **)malloc(sizeof(element_t *) * MAX_ELEMENTS);
    for (int i = 0; i < MAX_ELEMENTS; i++) {
        nouveau->elements[i] = (element_t *)malloc(sizeof(element_t));
        nouveau->elements[i]->buf =
            (char *)malloc(sizeof(char) * (strlen("EMPTY") + 1));
        nouveau->elements[i]->buf =
            strncpy(nouveau->elements[i]->buf, "EMPTY", (strlen("EMPTY") + 1));
        nouveau->elements[i]->timestamp = (unsigned long)time(NULL);
    }
    return nouveau;
}

int put_message(int region, long int timestamp, char *buf, int len) {
    if (region > MAX_ELEMENTS - 1) {
        log_error("Naughty tried to put message in excess position");
        return -1;
    }
    char *to_free = NULL;
    bool free_f = false;

    pthread_mutex_lock(&m[region]); // start of Critical Section
    if (timestamp > msg_store->elements[region]->timestamp) {
        msg_store->elements[region]->timestamp = timestamp;
        if (msg_store->elements[region]->buf != NULL) {
            to_free = msg_store->elements[region]->buf;
            free_f = true;
        }
        msg_store->elements[region]->buf = buf;
        msg_store->elements[region]->len = len;
        log_trace("New Element[%d] Value=\"%s\"", region,
                  msg_store->elements[region]->buf);
    }
    pthread_mutex_unlock(&m[region]); // end of Critical Section

    if (free_f) {
        free(to_free);
    }
    return 0;
}

element_t *get_message(int region) {
    if (region > MAX_ELEMENTS - 1) {
        log_error("Naughty tried to get message in excess position");
        return NULL;
    }
    return msg_store->elements[region];
}
