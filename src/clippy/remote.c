#include "remote.h"

void *accept_remote_client(void *args) {
    int socket, nbytes;
    char tosend[100];

    /* Unpack the arguments */
    socket = *(int *)args;

    pthread_detach(pthread_self());

    log_debug("Socket %d connected\n", socket);

    while (1) {
        sprintf(tosend, "%d -- Hello, socket!\n", (int)time(NULL));

        nbytes = send(socket, tosend, strlen(tosend), 0);

        if (nbytes == -1 && (errno == ECONNRESET || errno == EPIPE)) {
            log_debug("Socket %d disconnected\n", socket);
            close(socket);
            pthread_exit(NULL);
        } else if (nbytes == -1) {
            log_error("Unexpected error in send()");
            pthread_exit(NULL);
        }
        sleep(5);
    }

    pthread_exit(NULL);
}

void *remote_connection(void *args) {
    int serverSocket;
    int clientSocket;
    pthread_t worker_thread;
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage *clientAddr;
    socklen_t sinSize = sizeof(struct sockaddr_storage);
    char *portno = (char *)args;

    int *wa;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Listen on all interfaces
    if (getaddrinfo(NULL, portno, &hints, &res) != 0) {
        log_error("getaddrinfo() failed");
        pthread_exit(NULL);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
                -1) {
            log_error("Could not open socket on %s",
                      inet_ntoa(((struct sockaddr_in *)(p->ai_addr))->sin_addr));
            continue;
        }

        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
                -1) {
            log_error("Socket setsockopt() failed on %s",
                      inet_ntoa(((struct sockaddr_in *)(p->ai_addr))->sin_addr));
            close(serverSocket);
            continue;
        }

        if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1) {
            log_error("Socket bind() failed on %s",
                      inet_ntoa(((struct sockaddr_in *)(p->ai_addr))->sin_addr));
            close(serverSocket);
            continue;
        }

        if (listen(serverSocket, 5) == -1) {
            log_error("Socket listen() failed on %s",
                      inet_ntoa(((struct sockaddr_in *)(p->ai_addr))->sin_addr));
            close(serverSocket);
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        log_error("Could not find address to bind to");
        pthread_exit(NULL);
    }

    /* Loop and wait for connections */
    while (true) {
        /* Call accept(). The thread will block until a client establishes a
         * connection. */
        clientAddr = malloc(sinSize);
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)clientAddr,
                                   &sinSize)) == -1) {
            /* If this particular connection fails, no need to kill the entire thread.
             */
            free(clientAddr);
            perror("Could not accept() connection");
            continue;
        }

        wa = malloc(sizeof(int));
        *wa = clientSocket;

        if (pthread_create(&worker_thread, NULL, accept_remote_client, wa) != 0) {
            perror("Could not create a worker thread");
            free(clientAddr);
            free(wa);
            close(clientSocket);
            close(serverSocket);
            pthread_exit(NULL);
        }
    }

    pthread_exit(NULL);
}
