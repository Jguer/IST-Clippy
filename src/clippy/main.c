#include "comms.h"

#define N_ARGS 4

void usage(const char *name) { //_Verbose_OPT_* are debug only variables
    fprintf(stdout, "Example Usage: %s -c [ip] [port]\n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-c\t\t[server ip]\n"
            "\t\t\t[server port]\n");
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

    pthread_t remote_thread;
    pthread_t local_thread;
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

    if (pthread_create(&remote_thread, NULL, remote_connection, NULL) < 0) {
        log_error("Unable to create remote thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&local_thread, NULL, local_connection, NULL) < 0) {
        log_error("Unable to create a local thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(local_thread, NULL);
    pthread_join(remote_thread, NULL);

    pthread_exit(NULL);
}
