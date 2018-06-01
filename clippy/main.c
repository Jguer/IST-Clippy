#include "comms.h"

#define N_ARGS 4

void usage(const char *name) { //_Verbose_OPT_* are debug only variables
    fprintf(stdout, "Example Usage: %s -c [ip] [port] -d [clipboard dir]\n",
            name);
    fprintf(stdout, "Arguments:\n"
            "\t-c\t[server ip]"
            " [server port]\n"
            "\t-d\t[clipboard dir]\n");
}

/**
 * @brief Initialize sockets and handle incoming connections.
 */
void start_service(void) {
    pthread_t worker_thread;

    int local_socket = create_local_socket();
    if (local_socket == -1) {
        log_error("Could not bind local address");
        exit(EXIT_FAILURE);
    }
    int max_fd = local_socket;

    int remote_socket = create_remote_socket();
    if (remote_socket == -1) {
        log_error("Could not bind remote socket");
        exit(EXIT_FAILURE);
    }
    max_fd = (remote_socket > max_fd) ? remote_socket : max_fd;

    int sync_socket = establish_sync();
    if (sync_socket != -1) {
        pthread_mutex_lock(&remote_connections_mutex);
        list_push(remote_connections, sync_socket);
        pthread_mutex_unlock(&remote_connections_mutex);
        wa_t *wa = (wa_t *)malloc(sizeof(wa_t));
        wa->fd = sync_socket;
        wa->remote = true;
        if (pthread_create(&worker_thread, NULL, accept_client, wa) != 0) {
            free(wa);
            log_error("unable to create worker thread");
        }
    }

    fd_set readfds, testfds;
    struct sockaddr_in client_address;
    int client_socket;
    socklen_t client_len;

    FD_ZERO(&readfds);
    FD_SET(remote_socket, &readfds);
    FD_SET(local_socket, &readfds);
    FD_SET(STDIN_FILENO, &readfds);
    while (true) {
        int fd;
        testfds = readfds;
        int result = select(max_fd + 1, &testfds, (fd_set *)0, (fd_set *)0,
                            (struct timeval *)0);
        if (result < 1) {
            log_error("remote select error");
        }

        for (fd = 0; fd < max_fd + 1; fd++) {
            if (FD_ISSET(fd, &testfds)) {
                if (fd == STDIN_FILENO) {
                    char buf[4096];
                    int len = read(STDIN_FILENO, buf, 4096);
                    buf[len] = '\0';
                    if (strstr(buf, "exit") != NULL) {
                        log_info("user interrupt");
                        goto serverexit;
                    } else if (strstr(buf, "storage") != NULL) {
                        print_storage();
                    }
                } else if (fd == local_socket) {
                    client_len = sizeof(client_address);
                    client_socket =
                        accept(local_socket, (struct sockaddr *)&client_address,
                               (socklen_t *)&client_len);
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
                } else if (fd == remote_socket) {
                    client_len = sizeof(client_address);
                    client_socket =
                        accept(remote_socket, (struct sockaddr *)&client_address,
                               (socklen_t *)&client_len);
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
serverexit:
    close(remote_socket);
    close(local_socket);
}

int main(int argc, const char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-c") == 0) {
            if (argc > i + 2) {
                ip = argv[i + 1];
                portno = argv[i + 2];
                log_info("IP: %s PORT: %s", ip, portno);
                i = i + 2;
            } else {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            if (argc > i + 1) {
                dir = argv[i + 1];
                log_info("DIR: %s", dir);
                i = i + 1;
            }

        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    msg_store = new_storage();

    pthread_mutex_init(&remote_connections_mutex, NULL);
    pthread_mutex_init(&local_connections_mutex, NULL);

    for (int i = 0; i < MAX_ELEMENTS; i++) {
        pthread_mutex_init(&m[i], NULL);
        pthread_cond_init(&c[i], NULL);
    }

    sigset_t new;
    sigemptyset(&new);
    sigaddset(&new, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &new, NULL) != 0) {
        log_error("Unable to mask SIGPIPE");
        exit(EXIT_FAILURE);
    }

    start_service();
    free_storage(msg_store);
    list_clear(remote_connections);
    list_clear(local_connections);
}
