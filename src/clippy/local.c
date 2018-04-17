#include "local.h"

void *accept_local_client(void *args) {
    // Get the socket descriptor
    int *sock = (int *)args;
    int read_size;
    char *buf;
    header_t header;
    long int timestamp;

    // Receive a message from client
    while ((read_size = recv(*sock, &header, sizeof(header_t), 0)) > 0) {
        if (read_size != sizeof(header_t)) {
            log_error("Header unset - read_size: %d, expected: %d", read_size,
                      sizeof(header_t));
            continue;
        }
        timestamp = time(NULL);

        log_debug("HEADER Received OP: %d Region: %d Data_size: %d", header.op,
                  header.region, header.data_size);

        if (header.op == COPY) {
            log_info("Got COPY, waiting for message");
            buf = (char *)malloc(sizeof(char) * header.data_size);
            buf[0] = '\0';

            read_size = recv(*sock, buf, header.data_size, 0);
            if (read_size < header.data_size) {
                log_error("Received shorter message than expected");
            }
            buf[header.data_size] = '\0';
            log_debug("Received Message: %s", buf);

            if (put_message(header.region, timestamp, buf, header.data_size) == -1) {
                log_error("Failed to put message in storage");
                free(buf);
            }
        } else if (header.op == PASTE) {
            pthread_mutex_lock(&m[header.region]); // start of Critical Section
            element_t *data = get_message(header.region);
            size_t data_size = data->len;
            if (data_size > header.data_size) {
                data_size = header.data_size;
            }
            write(*sock, data->buf, data_size);
            pthread_mutex_unlock(&m[header.region]);
        } else if (header.op == WAIT) {
            pthread_mutex_lock(&m[header.region]); // start of Critical Section
            if (pthread_cond_wait(&c[header.region], &m[header.region]) != 0) {
                log_error("Unable to wait for paste");
            }
            element_t *data = get_message(header.region);
            size_t data_size = data->len;
            if (data_size > header.data_size) {
                data_size = header.data_size;
            }
            write(*sock, data->buf, data_size);

            pthread_mutex_unlock(&m[header.region]);
        }
    }

    if (read_size == 0) {
        log_info("Client %d disconnected", sock);
    } else if (read_size == -1) {
        log_error("Client %d recv failed", sock);
    }

    free(sock);
    pthread_exit(NULL);
}

void *local_connection(void *args) {
    int master_socket;
    struct sockaddr_un address;

    // create a master socket
    if ((master_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        log_fatal("local socket creation failed");
        exit(EXIT_FAILURE);
    }

    unlink(SOCK_PATH);

    // type of socket created
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCK_PATH);

    size_t len = strlen(address.sun_path) + sizeof(address.sun_family);

    if (bind(master_socket, (struct sockaddr *)&address, len) < 0) {
        log_fatal("local socket bind failed");
        pthread_exit(NULL);
    }
    log_info("Listener on path %s", SOCK_PATH);

    if (listen(master_socket, 5) < 0) {
        log_fatal("local socket listen failed");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    size_t addrlen = sizeof(address);
    log_info("Waiting Connections");

    while (true) {
        int *new_socket = malloc(sizeof(int));
        if ((*new_socket = accept(master_socket, (struct sockaddr *)&address,
                                  (socklen_t *)&addrlen)) < 0) {
            log_fatal("local socket accept error");
            pthread_exit(NULL);
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, accept_local_client, new_socket) < 0) {
            log_fatal("could not create thread");
            pthread_exit(NULL);
        }
        if (pthread_detach(thread_id) < 0) {
            log_fatal("could not detach thread");
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}
