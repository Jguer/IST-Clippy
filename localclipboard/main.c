#include "../utils/log.h"
#include <errno.h>
#include <pthread.h> //for threading , link with lpthread
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define N_ARGS 4
#define CLIPBOARD_NAME "CLIPBOARD_SOCKET"

void *connection_handler(void *);
int local_connection();

void usage(const char *name) { //_Verbose_OPT_* are debug only variables
    fprintf(stdout, "Example Usage: %s -c [ip] [port]\n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-c\t\t[server ip]\n"
            "\t\t\t[server port]\n");
}

int main(int argc, const char *argv[]) {
    int portno;
    char const *ip = NULL;

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

    log_info("IP: %s PORT: %d", ip, portno);

    if (local_connection() != 0) {
        log_error("Local connection error\n", ip, portno);
    }

    return 0;
}

int local_connection() {
    struct sockaddr_un local, client;
    unsigned int local_sock, client_sock;
    int len;

    // Create local socket
    local_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (local_sock == -1) {
        log_error("strerror(errno)");
    }

    // Bind socket
    local.sun_family = AF_UNIX; /* local is declared before socket() ^ */
    strncpy(local.sun_path, CLIPBOARD_NAME, strlen(CLIPBOARD_NAME));
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(local_sock, (struct sockaddr *)&local, len) < 0) {
        log_error("strerror(errno)");
    }
    log_info("Local socket open: %s", CLIPBOARD_NAME);

    listen(local_sock, 3);
    len = sizeof(struct sockaddr_un);
    pthread_t thread_id;

    while ((client_sock = accept(local_sock, (struct sockaddr *)&client,
                                 (socklen_t *)&len))) {

        if (pthread_create(&thread_id, NULL, connection_handler,
                           (void *)&client_sock) < 0) {
            log_error("Unable to create thread");
            return 1;
        }

        // Now join the thread , so that we dont terminate before the thread
        pthread_join(thread_id, NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }
    return 0;
}

/* Handle connection from each local client */
void *connection_handler(void *socket_desc) {
    // Get the socket descriptor
    int sock = *(int *)socket_desc;
    int read_size;
    char *message, client_message[2000];

    // Receive a message from client
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0) {
        // end of string marker
        client_message[read_size] = '\0';

        // Send the message back to client
        write(sock, client_message, strlen(client_message));

        // clear the message buffer
        memset(client_message, 0, 2000);
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    return 0;
}
