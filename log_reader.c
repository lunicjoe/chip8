#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 1) {
        fprintf(stderr, "log file needed\n");
        return EXIT_FAILURE;
    }
    FILE *log = fopen(argv[1], "r");
    if (log == NULL) {
        fprintf(stderr, "%s not found\n", argv[1]);
        return EXIT_FAILURE;
    }
    char c;
    while ((c = (char)fgetc(log)) != EOF) {
        fprintf(stdout, "%c", c);
    }
    fclose(log);
    return EXIT_SUCCESS;
}