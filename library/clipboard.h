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

/**
 * @brief OP describes the action on the server side
 */
enum op { COPY, PASTE, WAIT };

#pragma pack(1) // this helps to avoid serialization while sending over network.
/**
 * @brief Header provided to clipboard with metadata information.
 */
typedef struct header {
    enum op op;
    int region;
    size_t data_size;
    unsigned long timestamp;
    unsigned long hash;
} header_t;
#pragma pack(0) // turn packing off

/**
 * @brief Connect to a clipboard.
 *
 * @param char* Clipboard Directory
 *
 * @return Clipboard ID
 */
int clipboard_connect(char *);

/**
 * @brief Send a copy operation to the clipboard.
 *
 * @param int Clipboard ID
 * @param int Clipboard destination region
 * @param void* Buffer to copy
 * @param size_t Size of buffer
 *
 * @return Number of bytes copied to clipboard.
 */
int clipboard_copy(int, int, void *, size_t);

/**
 * @brief Send a paste operation to clipboard.
 *
 * @param int Clipboard ID
 * @param int Clipboard destination region
 * @param void* Buffer to copy into
 * @param size_t Size of buffer
 *
 * @return Number of bytes copied from clipboard.
 */
int clipboard_paste(int, int, void *, size_t);

/**
 * @brief Send a wait operation to clipboard.
 *
 * @param int Clipboard ID
 * @param int Clipboard destination region
 * @param void* Buffer to copy into
 * @param size_t Size of buffer
 *
 * @return Number of bytes copied from clipboard.
 */
int clipboard_wait(int, int, void *, size_t);

/**
 * @brief Close connection to the clipboard.
 *
 * @param int Clipboard ID
 */
void clipboard_close(int);

/**
 * @brief Generate djb2 hash of a buffer
 *
 * @param Buffer to hash
 * @param Size of buffer
 *
 * @return hash result
 */
unsigned long ht_hash(char const *, size_t);

#endif
