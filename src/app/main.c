#include "library/clipboard.h"

int main(int argc, char const *argv[]) {
    int clipboard_id = clipboard_connect("/tmp/CLIPBOARD_SOCKET");
    clipboard_copy(clipboard_id, 1, "hello fellow clipboard",
                   strlen("hello fellow clipboard") + 1);

    char buf[4096];
    clipboard_paste(clipboard_id, 1, buf, 4096);
    puts(buf);

    return 0;
}
