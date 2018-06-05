/*Lê texto do terminal e envia para o clipboard.*/

#include "../library/clipboard.h"

int main(int argc, char const *argv[]) {
    int clipboard_id = clipboard_connect("./CLIPBOARD_SOCKET");

    while (1) {
        char buf[4096];
        if (NULL == fgets(buf, 4096, stdin)) {
            return 1;
        }
        buf[strlen(buf) - 1] = '\0';

        if (strstr(buf, "exit") != NULL) {
            clipboard_close(clipboard_id);
            return 0;
        }
        clipboard_copy(clipboard_id, 1, buf, strlen(buf) + 1);
        clipboard_paste(clipboard_id, 1, buf, 4096);
    }
    clipboard_close(clipboard_id);
}
