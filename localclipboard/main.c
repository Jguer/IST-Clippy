#include "../utils/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define N_ARGS 4
#define CLIPBOARD_NAME "CLIPBOARD_SOCKET"
#define MESSAGE_SIZE 1024

void *accept_local_client(void *);
void *accept_remote_client(void *);
void *local_connection(void *);
void *remote_connection(void *);

void usage(const char *name) { //_Verbose_OPT_* are debug only variables
    fprintf(stdout, "Example Usage: %s -c [ip] [port]\n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-c\t\t[server ip]\n"
            "\t\t\t[server port]\n");
}

int main(int argc, const char *argv[]) {
    int portno;
    char const *ip = NULL;
    pthread_t remote_thread;
    pthread_t local_thread;

    if (argc < N_ARGS) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-c") == 0) {
        ip = argv[2];
        portno = atoi(argv[3]);
    } else {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    sigset_t new;
    sigemptyset(&new);
    sigaddset(&new, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &new, NULL) != 0) {
        log_error("Unable to mask SIGPIPE");
        exit(EXIT_FAILURE);
    }

    log_info("IP: %s PORT: %d", ip, portno);

    if (pthread_create(&remote_thread, NULL, local_connection, NULL) < 0) {
        log_error("Unable to create remote thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&local_thread, NULL, remote_connection, NULL) < 0) {
        log_error("Unable to create a local thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(remote_thread, NULL);
    pthread_join(local_thread, NULL);

    pthread_exit(NULL);
}

void *remote_handler(void *args) {
    int serverSocket;
    int clientSocket;
    pthread_t worker_thread;
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage *clientAddr;
    socklen_t sinSize = sizeof(struct sockaddr_storage);
    int *wa;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Return my address, so I can bind() to it

    /* Note how we call getaddrinfo with the host parameter set to NULL */
    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        perror("getaddrinfo() failed");
        pthread_exit(NULL);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
                -1) {
            perror("Could not open socket");
            continue;
        }

        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
                -1) {
            perror("Socket setsockopt() failed");
            close(serverSocket);
            continue;
        }

        if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1) {
            perror("Socket bind() failed");
            close(serverSocket);
            continue;
        }

        if (listen(serverSocket, 5) == -1) {
            perror("Socket listen() failed");
            close(serverSocket);
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "Could not find a socket to bind to.\n");
        pthread_exit(NULL);
    }

    /* Loop and wait for connections */
    while (1) {
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
        wa = clientSocket;

        if (pthread_create(&worker_thread, NULL, service_single_client, wa) != 0) {
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

void *remote_connection(void *args) {
    struct workerArgs *wa;
    int socket, nbytes;
    char tosend[100];

    /* Unpack the arguments */
    socket = (struct workerArgs *)wa->socket;

    pthread_detach(pthread_self());

    fprintf(stderr, "Socket %d connected\n", socket);

    while (1) {
        sprintf(tosend, "%d -- Hello, socket!\n", (int)time(NULL));

        nbytes = send(socket, tosend, strlen(tosend), 0);

        if (nbytes == -1 && (errno == ECONNRESET || errno == EPIPE)) {
            fprintf(stderr, "Socket %d disconnected\n", socket);
            close(socket);
            free(wa);
            pthread_exit(NULL);
        } else if (nbytes == -1) {
            perror("Unexpected error in send()");
            free(wa);
            pthread_exit(NULL);
        }
        sleep(5);
    }

    pthread_exit(NULL);
}
