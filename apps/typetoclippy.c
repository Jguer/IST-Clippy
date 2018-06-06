/*Lê texto do terminal e envia para o clipboard.*/

#include "../library/clipboard.h"

int main(int argc, char const *argv[]) {
    int clipboard_id = clipboard_connect("./CLIPBOARD_SOCKET");
    int size, region = 0;
    int nbytes = 0;
    char *buf = calloc(sizeof(char), 4096);

    printf("What type of function do you require? P(paste),C(copy) and "
           "W(wait)\n");
    while (1) {
        fflush(stdout);

        if (NULL == fgets(buf, 4096, stdin)) {
            return -1;
        }

        if (strstr(buf, "exit") != NULL) {
            clipboard_close(clipboard_id);
            return 0;
        } else if (strstr(buf, "P") != NULL) {
            printf("From which region do you want to paste?\n");
            if (scanf("%d", &region) < 0) {
                printf("Wrong arguments\n");
                break;
            }
            printf("What is the size of what you want to paste\n");
            if (scanf("%d", &size) < 0) {
                printf("Wrong arguments\n");
                break;
            }
            nbytes = clipboard_paste(clipboard_id, region, buf, size);
            printf("Return code: %d\n", nbytes);
            fwrite(buf, sizeof(char), 4096, stdout);
            printf("\n");
            /* puts(buf); */
        } else if (strstr(buf, "C") != NULL) {
            printf("Write a message to copied into the clipboard\n");
            if (NULL == fgets(buf, 4096, stdin)) {
                break;
            }
            printf("To what region do you want to copy?\n");
            if (scanf("%d", &region) < 0) {
                printf("Wrong arguments\n");
                break;
            }
            printf("What is the size of the message you want to copy\n");
            if (scanf("%d", &size) < 0) {
                printf("Wrong arguments\n");
                break;
            }
            nbytes = clipboard_copy(clipboard_id, region, buf, size);
            printf("Return code: %d\n", nbytes);
        } else if (strstr(buf, "W") != NULL) {
            printf("From which region do you want to paste?\n");
            if (scanf("%d", &region) < 0) {
                printf("Wrong arguments\n");
                break;
            }
            printf("What is the size of what you want to paste\n");
            if (scanf("%d", &size) < 0) {
                printf("Wrong arguments\n");
                break;
            }
            nbytes = clipboard_wait(clipboard_id, region, buf, size);
            printf("Return code: %d\n", nbytes);
            fwrite(buf, sizeof(char), 4096, stdout);
            printf("\n");
            /* puts(buf); */
        }
        memset(buf, 0, 4096);
        printf("What type of function do you require? P(paste),C(copy) and "
               "W(wait)\n");
    }
    clipboard_close(clipboard_id);
}
