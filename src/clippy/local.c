#include "local.h"

void *accept_local_client(void *args) {
    // Get the socket descriptor
    int sock = *(int *)args;
    int read_size;
    char client_message[MESSAGE_SIZE * 4];
    header_t got_header;
    long int timestamp;

    // Receive a message from client
    while ((read_size = recv(sock, &got_header, MESSAGE_SIZE, 0)) > 0) {
        if (read_size != sizeof(header_t)) {
            log_error("Header unset");
            continue;
        }
        timestamp = time(NULL);

        if (got_header.op == COPY) {
            log_info("Got COPY, waiting for message");
            char *buf = malloc(sizeof(char) * got_header.data_size);
            while ((read_size = recv(sock, client_message, MESSAGE_SIZE, 0)) <
                    got_header.data_size - 1) {
                buf = strncpy(buf + strlen(buf), client_message, read_size);
            }
            buf[got_header.data_size] = '\0';
            if (put_message(got_header.region, timestamp, buf,
                            got_header.data_size) == -1) {
                log_error("Failed to put message in storage");
            }
        } else if (got_header.op == PASTE) {
            element_t *data = get_message(got_header.region);
            write(sock, data->buf, data->len);
        }

        // clear the message buffer
        memset(client_message, 0, MESSAGE_SIZE);
    }

    if (read_size == 0) {
        log_info("Client disconnected");
    } else if (read_size == -1) {
        log_error("recv failed");
    }

    sock = 0;
    pthread_exit(NULL);
}

void *local_connection(void *args) {
    int master_socket;
    int client_socket[30];
    int max_clients = 30;
    struct sockaddr_un address;

    // set of socket descriptors
    fd_set readfds;

    // initialize all client_socket[] to 0 so not checked
    memset(client_socket, 0, sizeof(client_socket));

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
    printf("Listener on port %s \n", SOCK_PATH);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0) {
        log_fatal("local socket listen failed");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    size_t addrlen = sizeof(address);
    log_info("Waiting Connections");

    while (true) {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(master_socket, &readfds);
        int max_sd = master_socket;

        // add child sockets to set
        for (int i = 0; i < max_clients; i++) {
            // socket descriptor
            int sd = client_socket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // wait for an activity on one of the sockets , timeout is NULL , so wait
        // indefinitely
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            log_fatal("local socket select error");
        }

        // If something happened on the master socket , then its an incoming
        // connection
        if (FD_ISSET(master_socket, &readfds)) {
            int new_socket = 0;
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address,
                                     (socklen_t *)&addrlen)) < 0) {
                log_fatal("local socket accept error");
                pthread_exit(NULL);
            }

            // add new socket to array of sockets
            for (int i = 0; i < max_clients; i++) {
                // if position is empty
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    log_info("Adding to list of sockets as %d\n", i);

                    pthread_t thread_id;
                    if (pthread_create(&thread_id, NULL, accept_local_client,
                                       (void *)&client_socket[i]) < 0) {
                        log_fatal("could not create thread");
                        pthread_exit(NULL);
                    }
                    pthread_detach(thread_id);

                    break;
                }
            }
        }
    }
    pthread_exit(NULL);
}
