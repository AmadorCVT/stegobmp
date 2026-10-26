#include <stdio.h>
#include <string.h>

static void print_banner(void) {
    printf("stegobmp - Steganography tool\n");
    printf("Use --help for usage information.\n");
}

static void print_help(void) {
    printf("stegobmp - Steganography tool\n");
    printf("\n");
    printf("Usage:\n");
    printf("  stegobmp --help\n");
    printf("\n");
    printf("Planned operations (not implemented yet):\n");
    printf("  -embed     Hide a file inside a BMP image\n");
    printf("  -extract   Recover a hidden file from a BMP image\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_banner();
        return 0;
    }

    if (strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    fprintf(stderr, "stegobmp: unknown option '%s'\n", argv[1]);
    fprintf(stderr, "Use --help for usage information.\n");
    return 1;
}
