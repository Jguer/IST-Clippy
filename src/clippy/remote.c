#include "remote.h"

void *accept_remote_client(void *args) {
    int socket, nbytes;
    char tosend[100];

    /* Unpack the arguments */
    socket = *(int *)args;

    pthread_detach(pthread_self());

    log_debug("Socket %d connected\n", socket);

    while (true) {
        sprintf(tosend, "%d -- Hello, socket!\n", (int)time(NULL));

        nbytes = send(socket, tosend, strlen(tosend), 0);

        if (nbytes == -1 && (errno == ECONNRESET || errno == EPIPE)) {
            log_debug("Socket %d disconnected\n", socket);

            pthread_mutex_lock(&remote_connections_mutex);
            list_remove(remote_connections, socket);
            pthread_mutex_unlock(&remote_connections_mutex);
            close(socket);
            pthread_exit(NULL);
        } else if (nbytes == -1) {
            pthread_mutex_lock(&remote_connections_mutex);
            list_remove(remote_connections, socket);
            pthread_mutex_unlock(&remote_connections_mutex);
            log_error("Unexpected error in send()");
            pthread_exit(NULL);
        }
        sleep(5);
    }

    pthread_exit(NULL);
}

void *remote_connection(void *args) {
    int server_socket = -1;
    int client_socket;
    pthread_t worker_thread;
    struct sockaddr_in client_address;
    char *portno = (char *)args;
    struct ifaddrs *ifap, *ifa;
    char *addr;
    int client_len;

    fd_set readfds, testfds;
    srand(time(NULL));
    int local_port = (rand() % (50000 - 1500)) + 1500;

    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET) {

            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);

            struct sockaddr_in server_address;
            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = sa->sin_addr.s_addr;
            server_address.sin_port = htons(local_port);

            if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                log_info("Could not open socket %s \tAddress: %s", ifa->ifa_name, addr);
                server_socket = -1;
                continue;
            }
            // no wait time to reuse the socket
            int bReuseaddr = 1;
            setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
                       (const char *)&bReuseaddr, sizeof(bReuseaddr));

            if (bind(server_socket, (struct sockaddr *)&server_address,
                     sizeof(server_address)) == -1) {
                log_info("Could not bind: %sAddress %s with error %s", ifa->ifa_name,
                         addr, strerror(errno));
                close(server_socket);
                server_socket = -1;
                continue;
            }

            if (listen(server_socket, 5) == -1) {
                log_info("Could not listen: %s Address: %s", ifa->ifa_name, addr);
                close(server_socket);
                server_socket = -1;
                continue;
            }

            log_info("Listener on interface: %s Address: %s Port: %d\n",
                     ifa->ifa_name, addr, local_port);
            break;
        }
    }

    freeifaddrs(ifap);

    if (server_socket == -1) {
        log_error("Could not find address to bind to");
        pthread_exit(NULL);
    }

    FD_ZERO(&readfds);
    FD_SET(server_socket, &readfds);
    /* Loop and wait for connections */
    while (true) {
        int fd;

        testfds = readfds;
        int result = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0,
                            (struct timeval *)0);
        if (result < 1) {
            log_error("remote select error");
            pthread_exit(NULL);
        }

        for (fd = 0; fd < FD_SETSIZE; fd++) {
            // some data
            if (FD_ISSET(fd, &testfds)) {
                // new connection
                if (fd == server_socket) {
                    client_len = sizeof(client_address);
                    client_socket = accept(
                                        server_socket, (struct sockaddr *)&client_address, &client_len);
                    if (client_socket < 0) {
                        log_error("failed to accept connection");
                        continue;
                    }

                    pthread_mutex_lock(&remote_connections_mutex);
                    list_push(remote_connections, client_socket);
                    pthread_mutex_unlock(&remote_connections_mutex);
                    FD_SET(client_socket, &readfds);

                    log_trace("add client %d to remote_connections", client_socket);
                    if (pthread_create(&worker_thread, NULL, accept_remote_client,
                                       &client_socket) != 0) {
                        log_error("unable to create worker thread");
                    }
                }
            }
        }
    }

    pthread_exit(NULL);
}
