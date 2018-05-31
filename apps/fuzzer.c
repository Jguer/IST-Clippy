#include "../library/clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char *randstring(size_t length) {

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "0123456789,.-#'?!\0\n";
    char *randomString = NULL;

    if (length) {
        randomString = malloc(sizeof(char) * (length + 1));

        if (randomString) {
            for (int n = 0; n < length; n++) {
                int key = rand() % (int)(sizeof(charset) - 1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}

int main(int argc, char const *argv[]) {
    int sockfd = clipboard_connect("/tmp/CLIPBOARD_SOCKET");
    int reg;
    int msize;
    char buf[MESSAGE_SIZE * 2];
    char *sbuf = NULL;
    srand(time(NULL));

    while (true) {
        if (sbuf != NULL) {
            free(sbuf);
        }

        if ((rand() % 6) == 3) {
            clipboard_close(sockfd);
            sockfd = clipboard_connect("/tmp/CLIPBOARD_SOCKET");
        }

        msize = rand() % (6000 - 100) + 100;
        reg = rand() % 12;
        sbuf = randstring(msize);

        clipboard_copy(sockfd, reg, sbuf, msize);
        printf("Sent to %d\n", reg);
        fwrite(sbuf, msize, 1, stdout);

        reg = rand() % 12;
        int size = clipboard_paste(sockfd, reg, buf, msize);
        buf[size] = '\n';
        printf("Received from %d\n", reg);
        fwrite(sbuf, msize, 1, stdout);

        sleep(1);
    }

    clipboard_close(sockfd);
    return 0;
}
