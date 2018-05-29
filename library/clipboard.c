#include "clipboard.h"

#define UNIX_PATH_MAX 108

int ht_hash(char *s) {
    int a = 151;
    int m = 53;
    long hash = 0;
    const int len_s = (int)strlen(s);
    for (int i = 0; i < len_s; i++) {
        hash += (long)pow(a, len_s - (i + 1)) * s[i];
        hash = hash % m;
    }
    return (int)hash;
}

int clipboard_connect(char *clipboard_dir) {
    struct sockaddr_un address;
    int socket_fd;

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        /* log_fatal("Failed to socket(): %s", strerror(errno)); */
        return -1; // Error opening clipboard
    }

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, clipboard_dir, UNIX_PATH_MAX);

    if (connect(socket_fd, (struct sockaddr *)&address,
                sizeof(struct sockaddr_un)) != 0) {
        /* log_fatal("Failed to connect(): %s", strerror(errno)); */
        return -1;
    }

    return socket_fd; // No error found
}

int clipboard_copy(int clipboard_id, int region, void *buf, size_t count) {
    header_t header;

    header.op = COPY;
    header.region = region;
    header.data_size = count;
    header.hash = ht_hash(buf);
    header.timestamp = time(NULL);
    int nbytes;

    /* log_info("Header Information\tOP: %d\tRegion: %d\tData_size:%d", header.op,
     */
    /*          header.region, header.data_size); */
    if (send(clipboard_id, &header, sizeof(header_t), 0) < sizeof(header_t)) {
        /* log_warn("Failed to send(): %s with %s", buf, strerror(errno)); */
        return 0;
    }

    if ((nbytes = send(clipboard_id, buf, header.data_size, 0)) <
            header.data_size) {
        /* log_warn("Failed to send(): %s with %s", buf, strerror(errno)); */
        return 0;
    };

    return nbytes; // copy successful
}

int clipboard_paste(int clipboard_id, int region, void *buf, size_t count) {
    header_t header;
    header.op = PASTE;
    header.region = region;
    header.data_size = count;
    int nbytes;

    /* log_info("Header Information\tOP: %d\tRegion: %d\tData_size:%d", header.op,
     */
    /*          header.region, header.data_size); */

    if (send(clipboard_id, &header, sizeof(header_t), 0) < sizeof(header_t)) {
        /* log_warn("Failed to send(): %s with %s", buf, strerror(errno)); */
        return 0;
    }

    if ((nbytes = recv(clipboard_id, buf, MESSAGE_SIZE, 0)) == 0) {
        /* log_warn("Failed to recv(): %s with %s", buf, strerror(errno)); */
        return 0;
    };

    return nbytes; // copy successful
}

int clipboard_wait(int clipboard_id, int region, void *buf, size_t count) {
    header_t header;

    header.op = WAIT;
    header.region = region;
    header.data_size = count;
    ssize_t nbytes;

    if (send(clipboard_id, &header, sizeof(header_t), 0) <
            (ssize_t)sizeof(header_t)) {
        /* log_warn("Failed to send(): %s with %s", buf, strerror(errno)); */
        return 0;
    }
    nbytes = recv(clipboard_id, buf, MESSAGE_SIZE, 0);
    if (nbytes == -1) {
        /* log_warn("sd:%d unexpected error in recv(): %s with %s", clipboard_id,
         * buf, */
        /*          strerror(errno)); */
    }

    return (int)nbytes; // copy successful
}

void clipboard_close(int clipboard_id) {
    close(clipboard_id);
}
