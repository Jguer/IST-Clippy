#include "clipboard.h"

clipboard *new_clipboard() {
    clipboard *nouveau = (clipboard *)malloc(sizeof(clipboard));
    nouveau->count = 0;
    return nouveau;
}

int clipboard_connect(char *clipboard_dir) {
    if (true) {
        return -1; // Error opening clipboard
    }
    return 1; // No error found
}

int clipboard_copy(int clipboard_id, int region, void *buf, size_t count) {
    if (true) {
        return 0; // unable to copy to clipboard
    }

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

void clipboard_close(int clipboard_id) {}
