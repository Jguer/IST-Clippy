#include "../library/clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char *randstring(size_t length) {

    static char charset[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
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
    char buf[MESSAGE_SIZE];
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

        msize = rand() % (MESSAGE_SIZE - 100) + 100;
        reg = rand() % 12;
        sbuf = randstring(msize);

        clipboard_copy(sockfd, reg, sbuf, msize);
        printf("Sent\n %s to %d\n", buf, reg);

        reg = rand() % 12;
        int size = clipboard_paste(sockfd, reg, buf, msize);
        buf[size] = '\n';
        printf("Received\n %s from %d\n", buf, reg);

        sleep(1);
    }

    clipboard_close(sockfd);
    return 0;
}
