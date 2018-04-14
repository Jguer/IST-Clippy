#ifndef LOCAL_HEADER
#define LOCAL_HEADER
#include "library/clipboard.h"
#include "storage.h"
#include "utils/defs.h"
#include "utils/log.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Sockets */
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <pthread.h>

#define SOCK_PATH "/tmp/CLIPBOARD_SOCKET"

void *accept_local_client(void *);
void *local_connection(void *);

#endif
