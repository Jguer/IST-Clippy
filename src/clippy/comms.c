#include "comms.h"

typedef struct worker_arguments {
    int fd;
    bool remote;
} wa_t;

void *accept_client(void *args) {
    int nbytes;
    char buf[MAX_MESSAGE_SIZE];
    wa_t *wa = (wa_t *)args;

    pthread_detach(pthread_self());
    log_debug("socket %d connected", wa->fd);

    header_t header;
    long int timestamp;

    while (true) {
        nbytes = recv(wa->fd, &header, sizeof(header_t), 0);
        if (nbytes == 0) {
            log_debug("sd:%d socket disconnected", wa->fd);
            break;
        } else if (nbytes == -1) {
            log_error("sd:%d unexpected error in recv(): %s", strerror(errno),
                      wa->fd);
            break;
        } else if (nbytes != sizeof(header_t)) {
            log_error("sd: %d header unset - read_size: %d, expected: %d", nbytes,
                      sizeof(header_t), wa->fd);
            continue;
        }

        timestamp = time(NULL);

        log_debug("sd:%d HEADER Received OP: %d Region: %d Data_size: %d",
                  header.op, header.region, header.data_size, wa->fd);

        if (header.op == COPY) {
            if (header.data_size > MAX_MESSAGE_SIZE) {
                log_error(
                    "sd:%d Your message exceeds MAX_MESSAGE_SIZE. This incident will "
                    "be reported",
                    wa->fd);
            }

            nbytes = recv(wa->fd, buf, header.data_size, 0);
            if (nbytes < header.data_size) {
                log_error("sd:%d Received shorter message than expected", wa->fd);
            }

            buf[header.data_size] = '\0';

            if (put_message(header.region, timestamp, buf, header.data_size) == -1) {
                log_error("Failed to put message in storage");
            }
        } else if (header.op == PASTE) {
            pthread_mutex_lock(&m[header.region]); // start of Critical Section
            element_t *data = get_message(header.region);
            size_t data_size = data->len;
            if (data_size > header.data_size) {
                data_size = header.data_size;
            }
            write(wa->fd, data->buf, data_size);
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

            write(wa->fd, data->buf, data_size);
            pthread_mutex_unlock(&m[header.region]);
        }
    }

    if (wa->remote) {
        pthread_mutex_lock(&remote_connections_mutex);
        list_remove(remote_connections, wa->fd);
        pthread_mutex_unlock(&remote_connections_mutex);
    } else {
        pthread_mutex_lock(&local_connections_mutex);
        list_remove(local_connections, wa->fd);
        pthread_mutex_unlock(&local_connections_mutex);
    }
    close(wa->fd);
    free(wa);
    pthread_exit(NULL);
}

int create_remote_socket() {
    struct ifaddrs *ifap, *ifa;
    int server_socket = -1;
    char *addr;

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

            log_info("Listener on interface: %s Address: %s Port: %d", ifa->ifa_name,
                     addr, local_port);
            break;
        }
    }

    freeifaddrs(ifap);
    return server_socket;
}

int establish_sync() {
    int sockfd = -1, rc, i, port, nBytes;
    struct sockaddr_in localAddr, servAddr;
    struct hostent *h;
    port = atoi(portno);
    if (port <= 0 || port > 65535) // check number of TCP server port
    {
        log_error("The port number given is wrong");
        return -1;
    }

    h = gethostbyname(ip);
    if (h == NULL) // check assigment of TCP server host
    {
        log_error("unknown host");
        return -1;
    }

    /* Create TCP socket */
    servAddr.sin_family = h->h_addrtype;
    memcpy((char *)&servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servAddr.sin_port = htons(port);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) // check TCP socket is created correctly
    {
        perror("Cannot open socket ");
        return -1;
    }

    /* Bind any port number */

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    rc = bind(sockfd, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (rc < 0) // check TCP socket is bind correctly
    {
        perror("Cannot bind on TCP port ");
        close(sockfd);
        return -1;
    }

    rc = connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (rc < 0) // check TCP socket is connected correctly
    {
        perror("Cannot connect ");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void *remote_connection(void *args) {
    pthread_t worker_thread;
    struct sockaddr_in client_address;
    int client_socket;
    int client_len;
    fd_set readfds, testfds;

    int server_socket = create_remote_socket();
    if (server_socket == -1) {
        log_error("Could not find address to bind to");
        pthread_exit(NULL);
    }

    int clipboard_socket = establish_sync();
    if (clipboard_socket == -1) {
        log_error("Unable to connect to foreign clipboard");
    }

    FD_ZERO(&readfds);
    FD_SET(server_socket, &readfds);
    /* Loop and wait for connections */
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        int fd;
        int max_fd = server_socket;
        pthread_mutex_lock(&remote_connections_mutex);
        list_each_elem(remote_connections, elem) {
            if (*elem > 0) {
                FD_SET(*elem, &readfds);
            }
            if (*elem > max_fd) {
                max_fd = *elem;
            }
        }
        pthread_mutex_unlock(&remote_connections_mutex);

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

                    wa_t *wa = (wa_t *)malloc(sizeof(wa_t));
                    wa->fd = client_socket;
                    wa->remote = true;
                    if (pthread_create(&worker_thread, NULL, accept_client, wa) != 0) {
                        free(wa);
                        log_error("unable to create worker thread");
                    }
                }
            }
        }
    }

    close(server_socket);
    pthread_exit(NULL);
}

int create_local_socket() {
    struct sockaddr_un address;
    int server_socket = -1;
    // create a master socket
    if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        log_fatal("local socket creation failed");
        exit(EXIT_FAILURE);
    }

    unlink(SOCK_PATH);

    // type of socket created
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCK_PATH);

    size_t len = strlen(address.sun_path) + sizeof(address.sun_family);

    if (bind(server_socket, (struct sockaddr *)&address, len) < 0) {
        log_fatal("local socket bind failed");
        pthread_exit(NULL);
    }
    log_info("Listener on path %s", SOCK_PATH);

    if (listen(server_socket, 5) < 0) {
        log_fatal("local socket listen failed");
        exit(EXIT_FAILURE);
    }
    log_info("Waiting Connections");
    return server_socket;
}

void *local_connection(void *args) {
    pthread_t worker_thread;
    struct sockaddr_in client_address;
    int client_socket;
    int client_len;
    fd_set readfds, testfds;

    int server_socket = create_local_socket();
    /* Loop and wait for connections */
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        int max_fd = server_socket;
        pthread_mutex_lock(&local_connections_mutex);
        list_each_elem(local_connections, elem) {
            if (*elem > 0) {
                FD_SET(*elem, &readfds);
            }
            if (*elem > max_fd) {
                max_fd = *elem;
            }
        }
        pthread_mutex_unlock(&local_connections_mutex);

        testfds = readfds;
        int activity = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0,
                              (struct timeval *)0);
        if ((activity < 0) && (errno != EINTR)) {
            log_error("local select error %s", strerror(errno));
            pthread_exit(NULL);
        }

        for (int fd = 0; fd < FD_SETSIZE; fd++) {
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

                    pthread_mutex_lock(&local_connections_mutex);
                    list_push(local_connections, client_socket);
                    pthread_mutex_unlock(&local_connections_mutex);

                    wa_t *wa = (wa_t *)malloc(sizeof(wa_t));
                    wa->fd = client_socket;
                    wa->remote = false;
                    if (pthread_create(&worker_thread, NULL, accept_client, wa) != 0) {
                        free(wa);
                        log_error("unable to create worker thread");
                    }
                }
            }
        }
    }
    close(server_socket);
    pthread_exit(NULL);
}
