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
    header_t header;

    header.op = COPY;
    header.region = region;
    header.data_size = count;

    log_info("Header Information\tOP: %d\tRegion: %d\tData_size:%d", header.op,
             header.region, header.data_size);
    if (send(clipboard_id, &header, sizeof(header_t), 0) < sizeof(header_t)) {
        log_warn("Failed to send(): %s with %s", buf, strerror(errno));
        return 0;
    }

    if (send(clipboard_id, buf, header.data_size, 0) < header.data_size) {
        log_warn("Failed to send(): %s with %s", buf, strerror(errno));
        return 0;
    };

    return header.data_size; // copy successful
}

int clipboard_paste(int clipboard_id, int region, void *buf, size_t count) {
    header_t header;
    header.op = PASTE;
    header.region = region;
    header.data_size = count;
    int nbytes;

    log_info("Header Information\tOP: %d\tRegion: %d\tData_size:%d", header.op,
             header.region, header.data_size);

    if (send(clipboard_id, &header, sizeof(header_t), 0) < sizeof(header_t)) {
        log_warn("Failed to send(): %s with %s", buf, strerror(errno));
        return 0;
    }

    if ((nbytes = recv(clipboard_id, buf, MESSAGE_SIZE, 0)) == 0) {
        log_warn("Failed to recv(): %s with %s", buf, strerror(errno));
        return 0;
    };

    return nbytes; // copy successful
}

int clipboard_wait(int clipboard_id, int region, void *buf, size_t count) {
    header_t header;

    header.op = WAIT;
    header.region = region;
    header.data_size = count;
    int nbytes;

    if (send(clipboard_id, &header, sizeof(header_t), 0) < sizeof(header_t)) {
        log_warn("Failed to send(): %s with %s", buf, strerror(errno));
        return 0;
    }
    nbytes = recv(clipboard_id, buf, MESSAGE_SIZE, 0);
    if (nbytes == -1) {
        log_warn("sd:%d unexpected error in recv(): %s with %s", clipboard_id, buf,
                 strerror(errno));
    }

    return nbytes; // copy successful
}

void clipboard_close(int clipboard_id) {
    close(clipboard_id);
}
