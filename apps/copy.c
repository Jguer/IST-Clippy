/*
Copia tres strings diferentes para 3 regioes diferentes
*/

#include "../library/clipboard.h"

int main(int argc, char const *argv[]) {
    if (argc < 3) {
        printf("Usage: %s [region ID] [message]\n", argv[0]);
        return 1;
    }

    int region = atoi(argv[1]);
    char *buf = (char *)argv[2];

    int clipboard_id = clipboard_connect("./CLIPBOARD_SOCKET");

    clipboard_copy(clipboard_id, region, buf, strlen(buf) + 1);
    /* fwrite(buf, sizeof(char), 4096, stdin); */

    clipboard_close(clipboard_id);
    return 0;
}
