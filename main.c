#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N_ARGS 4

int OUTPUT_FD;

void usage(const char *name) { //_Verbose_OPT_* are debug only variables
    fprintf(OUTPUT_FD, "Example Usage: %s -c [ip] [port]\n", name);
    fprintf(OUTPUT_FD, "Arguments:\n"
            "\t-c\t\t[server ip]\n"
            "\t\t\t[server port]\n");
}

int main(int argc, const char *argv[]) {
    int OUTPUT_FD = stdout;
    int portno;
    char const *ip = NULL;

    if (argc < N_ARGS) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-c") == 0) {
        ip = argv[2];
        portno = atoi(argv[3]);
    } else {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    fprintf(OUTPUT_FD, "IP: %s PORT: %d\n", ip, portno);
}
