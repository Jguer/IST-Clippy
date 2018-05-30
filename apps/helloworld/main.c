#include "../../library/clipboard.h"

int main(int argc, char const *argv[]) {
    int clipboard_id = clipboard_connect("/tmp/CLIPBOARD_SOCKET");
    clipboard_copy(clipboard_id, 1, "hello fellow clipboard",
                   strlen("hello fellow clipboard") + 1);

    clipboard_copy(clipboard_id, 5, "hello clipboard",
                   strlen("hello fellow clipboard") + 1);

    clipboard_copy(clipboard_id, 2, "hello fellow",
                   strlen("hello fellow clipboard") + 1);

    clipboard_copy(clipboard_id, 3, "hello clipfellow",
                   strlen("hello fellow clipboard") + 1);
    char buf[4096];
    clipboard_paste(clipboard_id, 1, buf, 4096);
    puts(buf);

    clipboard_paste(clipboard_id, 2, buf, 4096);
    puts(buf);

    clipboard_paste(clipboard_id, 3, buf, 4096);
    puts(buf);

    clipboard_close(clipboard_id);
    return 0;
}
