/*
Copia tres strings diferentes para 3 regioes diferentes
*/

#include "../library/clipboard.h"

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [region ID]\n", argv[0]);
        return 1;
    }

    char buf[4096];
    int region = atoi(argv[1]);

    int clipboard_id = clipboard_connect("./CLIPBOARD_SOCKET");

    clipboard_wait(clipboard_id, region, buf, 4096);
    fwrite(buf, sizeof(char), 4096, stdin);

    clipboard_close(clipboard_id);
    return 0;
}
