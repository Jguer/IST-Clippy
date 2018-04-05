#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_ARGS 2

int main(int argc, char const *argv[]) {
    char *port = NULL;
    char *ip = NULL;

    if (argc < N_ARGS) {
        char error[64];
        perror(sprintf(error, "Usage: %s -c <ip address> <port>\n", argv[0]));
        return 0;
    }
}
