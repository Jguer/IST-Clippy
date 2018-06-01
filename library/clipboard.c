#include "clipboard.h"

#define UNIX_PATH_MAX 108

/* int ht_hash(char *s, int len_s) { */
/*     int a = 151; */
/*     int m = 53; */
/*     long hash = 0; */
/*     for (int i = 0; i < len_s; i++) { */
/*         hash += (long)pow(a, len_s - (i + 1)) * s[i]; */
/*         hash = hash % m; */
/*     } */
/*     return (int)hash; */
/* } */

unsigned long ht_hash(char const *s, size_t len_s) {
    unsigned long hash = 5381;

    for (int i = 0; i < len_s; i++) {
        unsigned long c = s[i];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
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
    header.hash = ht_hash(buf, count);
    header.timestamp = time(NULL);
    int nbytes;

    /* log_info("Header Information\tOP: %d\tRegion: %d\tData_size:%d",
     * header.op,
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

int clipboard_pasta(int clipboard_id, void *buf, header_t header) {
    ssize_t nbytes;
    int count = header.data_size;
    if (send(clipboard_id, &header, sizeof(header_t), 0) < sizeof(header_t)) {
        /* log_warn("Failed to send(): %s with %s", buf, strerror(errno)); */
        return 0;
    }

    nbytes = recv(clipboard_id, &header, sizeof(header_t), MSG_WAITALL);
    if (nbytes == 0 || nbytes == -1 || nbytes != sizeof(header_t)) {
        return 0;
    }

    void *rbuf = malloc(header.data_size);

    nbytes = recv(clipboard_id, rbuf, header.data_size, MSG_WAITALL);
    if (nbytes == 0 || nbytes == -1 || nbytes != header.data_size) {
        return 0;
    }

    int data_size = (header.data_size > count) ? count : header.data_size;
    memcpy(buf, rbuf, data_size);

    free(rbuf);
    return (int)nbytes;
}
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count) {
    header_t header;
    header.op = PASTE;
    header.region = region;
    header.data_size = count;

    return clipboard_pasta(clipboard_id, buf, header);
}

int clipboard_wait(int clipboard_id, int region, void *buf, size_t count) {
    header_t header;
    header.op = WAIT;
    header.region = region;
    header.data_size = count;

    return clipboard_pasta(clipboard_id, buf, header);
}

void clipboard_close(int clipboard_id) {
    close(clipboard_id);
}
