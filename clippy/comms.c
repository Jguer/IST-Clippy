#include "comms.h"
#define STDIN 0

typedef struct worker_arguments {
    int fd;
    bool remote;
} wa_t;

void *accept_client(void *args) {
    char buf[MAX_MESSAGE_SIZE];
    wa_t *wa = (wa_t *)args;

    pthread_detach(pthread_self());
    /* log_debug("socket %d connected", wa->fd); */

    header_t header;
    unsigned long timestamp;

    while (true) {
        int nbytes = recv(wa->fd, &header, sizeof(header_t), 0);
        if (nbytes == 0) {
            log_info("socket %d disconnected", wa->fd);
            break;
        } else if (nbytes == -1) {
            log_error("sd:%d unexpected error in recv(): %s", strerror(errno),
                      wa->fd);
            break;
        } else if (nbytes != sizeof(header_t)) {
            log_error("sd: %d header unset - read_size: %d, expected: %d", wa->fd,
                      nbytes, sizeof(header_t));
            continue;
        }

        /* log_debug("sd:%d HEADER Received OP: %d Region: %d Data_size: %d",
         * wa->fd, */
        /*           header.op, header.region, header.data_size, wa->fd); */

        if (header.op == COPY) {
            timestamp = time(NULL);
            if (timestamp < header.timestamp) {
                log_error("sd:%d has an invalid timestamp. Timestamp overwritten. This "
                          "incident will "
                          "be reported",
                          wa->fd);
                header.timestamp = timestamp;
            }

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
            int calc_hash = ht_hash(buf);
            if (calc_hash != header.hash) {
                log_error("sd:%d Hash does not correspond to header", wa->fd);
                header.hash = calc_hash;
            }

            if (put_message(header.region, header.timestamp, header.hash,
                            header.data_size, buf) == -1) {
                log_error("Failed to put message in storage");
            }

            pthread_mutex_lock(&remote_connections_mutex);
            list_each_elem(remote_connections, elem) {
                if (*elem != wa->fd) {
                    clipboard_copy(*elem, header.region, buf, header.data_size);
                }
            }
            pthread_mutex_unlock(&remote_connections_mutex);

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
        } else {
            log_error("sd: %d invalid OP %d", wa->fd, header.op);
            continue;
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
    int local_port = (rand() % (8100 - 8000)) + 8000;

    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET) {
            if (strstr(ifa->ifa_name, "lo")) {
                printf("Skipping %s", ifa->ifa_name);
                continue;
            }

            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);

            struct sockaddr_in server_address;
            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = sa->sin_addr.s_addr;
            server_address.sin_port = htons(local_port);

            if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                log_error("Could not open socket %s \tAddress: %s", ifa->ifa_name,
                          addr);
                server_socket = -1;
                continue;
            }
            // no wait time to reuse the socket
            int bReuseaddr = 1;
            setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
                       (const char *)&bReuseaddr, sizeof(bReuseaddr));

            if (bind(server_socket, (struct sockaddr *)&server_address,
                     sizeof(server_address)) == -1) {
                log_error("Could not bind: %sAddress %s with error %s", ifa->ifa_name,
                          addr, strerror(errno));
                close(server_socket);
                server_socket = -1;
                continue;
            }

            if (listen(server_socket, 5) == -1) {
                log_error("Could not listen: %s Address: %s", ifa->ifa_name, addr);
                close(server_socket);
                server_socket = -1;
                continue;
            }

            log_info("Listener on interface: %s Address: %s %d", ifa->ifa_name, addr,
                     local_port);
            break;
        }
    }

    freeifaddrs(ifap);
    return server_socket;
}

int establish_sync() {
    int sockfd = -1, port;
    struct sockaddr_in localAddr, servAddr;
    struct hostent *h;
    if (portno == NULL || ip == NULL) {
        return -1;
    }

    port = atoi(portno);
    if (port <= 0 || port > 65535) // check number of TCP server port
    {
        log_error("The port number given is wrong");
        return -1;
    }

    h = gethostbyname(ip);
    if (h == NULL) // check assigment of TCP server host
    {
        log_error("Unknown host");
        return -1;
    }

    /* Create TCP socket */
    servAddr.sin_family = h->h_addrtype;
    memcpy((char *)&servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servAddr.sin_port = htons(port);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log_error("Could not open sync socket");
        return -1;
    }

    /* Bind any port number */

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    if (bind(sockfd, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) {
        log_error("Could not bind sync socket");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        log_error("Unable to connect to sync socket");
        close(sockfd);
        return -1;
    }

    sleep(1);

    log_info("Starting initial sync with foreign clipboard");
    char buf[MAX_MESSAGE_SIZE];
    for (int i = 0; i < MAX_ELEMENTS; i++) {
        int nbytes = clipboard_paste(sockfd, i, &buf, MAX_MESSAGE_SIZE);
        buf[nbytes] = '\0';
        if (put_message(i, 1, 0, nbytes, buf) == -1) {
            log_error("Failed to put message in storage");
        }
    }

    return sockfd;
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
