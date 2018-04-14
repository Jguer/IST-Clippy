#ifndef REMOTE_HEADER
#define REMOTE_HEADER

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

/* Remote Connection */
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <pthread.h>

void *accept_remote_client(void *);
void *remote_connection(void *);

#endif
