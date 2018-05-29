#include "comms.h"

#define N_ARGS 4

void usage(const char *name) { //_Verbose_OPT_* are debug only variables
    fprintf(stdout, "Example Usage: %s -c [ip] [port]\n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-c\t\t[server ip]\n"
            "\t\t\t[server port]\n");
}

typedef struct worker_arguments {
    int fd;
    bool remote;
} wa_t;

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
                    char buf[MAX_MESSAGE_SIZE];
                    int len = read(STDIN_FILENO, buf, MAX_MESSAGE_SIZE);
                    buf[len] = '\0';
                    if (strstr(buf, "exit") != NULL) {
                        log_info("user interrupt");
                        goto serverexit;
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
    if (argc > 3) {
        if (strcmp(argv[1], "-c") == 0) {
            ip = argv[2];
            portno = argv[3];
            log_info("IP: %s PORT: %s", ip, portno);
        } else if (strcmp(argv[1], "-h") == 0) {
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
