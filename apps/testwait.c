

#include "../library/clipboard.h"

int main(int argc, char const *argv[]) {
    int clipboard_id = clipboard_connect("/tmp/CLIPBOARD_SOCKET");
    char buf[4096];
    clipboard_paste(clipboard_id, 2, buf, 4096);
    puts(buf);

    clipboard_wait(clipboard_id, 1, buf, 4096);
    puts(buf);

    clipboard_close(clipboard_id);
    return 0;
}
