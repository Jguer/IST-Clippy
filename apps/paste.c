/*
Copia tres strings diferentes para 3 regioes diferentes
*/

#include "../library/clipboard.h"

int main(int argc, char const *argv[]) {
    if (argc < 3) {
        printf("Usage: %s [region ID] [size of buffer]\n", argv[0]);
        return 1;
    }

    int region = atoi(argv[1]);
    int size = atoi(argv[2]);
    char *buf = malloc(sizeof(char) * size);

    int clipboard_id = clipboard_connect("./CLIPBOARD_SOCKET");

    clipboard_paste(clipboard_id, region, buf, size);
    fwrite(buf, sizeof(char), size, stdout);

    clipboard_close(clipboard_id);
    return 0;
}
