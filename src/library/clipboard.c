#include "clipboard.h"

#define UNIX_PATH_MAX 108

int clipboard_connect(char *clipboard_dir) {
    struct sockaddr_un address;
    int socket_fd;

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        log_fatal("Failed to socket(): %s", strerror(errno));
        return -1; // Error opening clipboard
    }

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, UNIX_PATH_MAX, clipboard_dir);

    if (connect(socket_fd, (struct sockaddr *)&address,
                sizeof(struct sockaddr_un)) != 0) {
        log_fatal("Failed to connect(): %s", strerror(errno));
        return -1;
    }

    return socket_fd; // No error found
}

int clipboard_copy(int clipboard_id, int region, void *buf, size_t count) {
    int nbytes;
    char buffer[256];

    nbytes = snprintf(buffer, 256, "hello from a client");
    write(clipboard_id, buffer, nbytes);

    nbytes = read(clipboard_id, buffer, 256);
    if (true) {
        return 0; // unable to copy to clipboard
    }

    buffer[nbytes] = 0;

    printf("MESSAGE FROM SERVER: %s\n", buffer);

    return 1; // copy successful
}

int clipboard_paste(int clipboard_id, int region, void *buf, size_t count) {
    if (true) {
        return 0; // unable to paste from clipboard
    }

    return 1; // copy successful
}

int clipboard_wait(int clipboard_id, int region, void *buf, size_t count) {
    if (true) {
        return 0; // unable to paste from clipboard
    }

    return 1; // copy successful
}

void clipboard_close(int clipboard_id) {
    close(clipboard_id);
}
