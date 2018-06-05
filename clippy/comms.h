#ifndef _REMOTE_H
#define _REMOTE_H

#include "storage.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * @brief Default local socket location
 */
#define SOCK_PATH "./CLIPBOARD_SOCKET"

/**
 * @brief Port number for sync clipboard.
 */
char const *portno;

/**
 * @brief Remote IP for sync clipboard.
 */
char const *ip;

/**
 * @brief Clipboard dir
 */
char const *dir;

/**
 * @brief Worker Arguments for accept_client.
 * @see accept_client
 *
 */
typedef struct worker_arguments {
    int fd;      /**< socket file descriptor. */
    bool remote; /**< is remote socket */
} wa_t;

/**
 * @brief Handle remote and local client communication.
 *
 * @param wa_t Worker arguments for thread
 * @see worker_arguments
 *
 * @return exit status
 */
void *accept_client(void *);
/**
 * @brief Create local connection socket.
 *
 * @return socket fd
 */
int create_local_socket(void);
/**
 * @brief Create remote connection socket.
 *
 * @return socket fd
 */
int create_remote_socket(void);
/**
 * @brief Connect to external clipboard.
 *
 * @return external clipboard fd.
 */
int establish_sync(void);

#endif
