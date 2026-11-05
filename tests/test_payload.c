/*
 * test_payload.c - payload build/parse, extension handling and capacity.
 * Run with `make test`.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"
#include "payload.h"
#include "steg.h"
#include "stegobmp.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        checks++;                                                            \
        if (!(cond)) {                                                       \
            failures++;                                                      \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
        }                                                                    \
    } while (0)

static void hexdump(const char *label, const uint8_t *p, size_t n) {
    printf("  %s:", label);
    for (size_t i = 0; i < n && i < 16; i++) {
        printf(" %02X", p[i]);
    }
    printf("%s\n", n > 16 ? " ..." : "");
}

static Options plain_options(void) {
    Options o;
    memset(&o, 0, sizeof o);
    o.op = OP_EMBED;
    o.method = STEG_LSB1;
    o.algo = CIPHER_AES128;
    o.mode = MODE_CBC;
    return o;
}

/* Build + parse without encryption and compare the content. */
static void roundtrip(const char *name, const uint8_t *file, size_t size,
                      const char *ext) {
    printf("%s\n", name);
    Options opts = plain_options();
    uint8_t *stream = NULL, *back = NULL;
    size_t stream_size = 0, back_size = 0;
    char back_ext[PAYLOAD_EXT_MAX];

    CHECK(payload_build(file, size, ext, &opts, &stream, &stream_size) ==
          STEGO_OK);
    CHECK(stream_size == 4 + size + strlen(ext) + 1);
    hexdump("stream", stream, stream_size);

    CHECK(payload_parse(stream, stream_size, &opts, &back, &back_size,
                        back_ext) == STEGO_OK);
    CHECK(back_size == size);
    CHECK(size == 0 || memcmp(back, file, size) == 0);
    CHECK(strcmp(back_ext, ext) == 0);

    /* Extra bytes after the payload (the rest of the carrier) are ignored. */
    size_t padded_size = stream_size + 100;
    uint8_t *padded = calloc(padded_size, 1);
    memcpy(padded, stream, stream_size);
    memset(padded + stream_size, 0xA5, 100);
    free(back);
    back = NULL;
    CHECK(payload_parse(padded, padded_size, &opts, &back, &back_size,
                        back_ext) == STEGO_OK);
    CHECK(back_size == size && strcmp(back_ext, ext) == 0);

    free(padded);
    free(back);
    free(stream);
}

static void test_roundtrips(void) {
    const char text[] = "Hola, mundo\n";
    roundtrip("text", (const uint8_t *)text, strlen(text), ".txt");

    uint8_t bin[1000];
    for (size_t i = 0; i < sizeof bin; i++) {
        bin[i] = (uint8_t)(i * 31 + 7);   /* includes 0x00 bytes */
    }
    bin[10] = 0;
    roundtrip("binary", bin, sizeof bin, ".bin");

    roundtrip("empty file", (const uint8_t *)"", 0, ".txt");
}

static void test_size_byte_order(void) {
    printf("size byte order\n");
    Options opts = plain_options();

    /* Example of the assignment: 17118 -> 00 00 42 DE */
    size_t size = 17118;
    uint8_t *file = calloc(size, 1);
    uint8_t *stream = NULL;
    size_t stream_size = 0;
    CHECK(payload_build(file, size, ".png", &opts, &stream, &stream_size) ==
          STEGO_OK);
    hexdump("stream", stream, stream_size);
    CHECK(stream[0] == 0x00 && stream[1] == 0x00);
    CHECK(stream[2] == 0x42 && stream[3] == 0xDE);
    /* ... and the extension ".png\0" sits at the end */
    CHECK(memcmp(stream + 4 + size, ".png", 5) == 0);
    free(stream);
    free(file);

    /* Small case, exact bytes: "AB" + ".txt" */
    CHECK(payload_build((const uint8_t *)"AB", 2, ".txt", &opts, &stream,
                        &stream_size) == STEGO_OK);
    const uint8_t expected[] = {0x00, 0x00, 0x00, 0x02, 'A', 'B',
                                '.', 't', 'x', 't', 0x00};
    hexdump("stream", stream, stream_size);
    CHECK(stream_size == sizeof expected);
    CHECK(memcmp(stream, expected, sizeof expected) == 0);
    free(stream);
}

static void test_corrupt(void) {
    printf("corrupt data\n");
    Options opts = plain_options();
    uint8_t *file = NULL;
    size_t file_size = 0;
    char ext[PAYLOAD_EXT_MAX];

    /* declared size larger than the carrier */
    const uint8_t too_big[] = {0x00, 0x10, 0x00, 0x00, 'x', '.', 't', 0};
    CHECK(payload_parse(too_big, sizeof too_big, &opts, &file, &file_size,
                        ext) == STEGO_ERR_CORRUPT);
    printf("  msg: %s\n", stego_error_message());

    /* extension without '\0' */
    const uint8_t no_nul[] = {0x00, 0x00, 0x00, 0x01, 'x', '.', 't', 'x', 't'};
    CHECK(payload_parse(no_nul, sizeof no_nul, &opts, &file, &file_size,
                        ext) == STEGO_ERR_CORRUPT);

    /* extension not starting with '.' */
    const uint8_t no_dot[] = {0x00, 0x00, 0x00, 0x01, 'x', 't', 'x', 't', 0};
    CHECK(payload_parse(no_dot, sizeof no_dot, &opts, &file, &file_size,
                        ext) == STEGO_ERR_CORRUPT);

    /* path separators are never accepted in the extension */
    const uint8_t slash[] = {0x00, 0x00, 0x00, 0x00, '.', '.', '/', 'x', 0};
    CHECK(payload_parse(slash, sizeof slash, &opts, &file, &file_size,
                        ext) == STEGO_ERR_CORRUPT);

    /* random-looking data: huge size */
    const uint8_t noise[] = {0xDE, 0xAD, 0xBE, 0xEF, 1, 2, 3, 4};
    CHECK(payload_parse(noise, sizeof noise, &opts, &file, &file_size,
                        ext) == STEGO_ERR_CORRUPT);

    /* shorter than the size field, and empty */
    CHECK(payload_parse(noise, 3, &opts, &file, &file_size, ext) ==
          STEGO_ERR_CORRUPT);
    CHECK(payload_parse(noise, 0, &opts, &file, &file_size, ext) ==
          STEGO_ERR_CORRUPT);
}

static void test_extension(void) {
    printf("extension from path\n");
    char ext[PAYLOAD_EXT_MAX];

    CHECK(payload_ext_from_path("mensaje1.txt", ext) == STEGO_OK);
    CHECK(strcmp(ext, ".txt") == 0);
    CHECK(payload_ext_from_path("dir.v2/a.tar.gz", ext) == STEGO_OK);
    CHECK(strcmp(ext, ".gz") == 0);
    CHECK(payload_ext_from_path("C:\\x.y\\foto.PNG", ext) == STEGO_OK);
    CHECK(strcmp(ext, ".PNG") == 0);

    CHECK(payload_ext_from_path("README", ext) == STEGO_ERR_ARGS);
    printf("  msg: %s\n", stego_error_message());
    CHECK(payload_ext_from_path("dir.v2/README", ext) == STEGO_ERR_ARGS);
    CHECK(payload_ext_from_path(".bashrc", ext) == STEGO_ERR_ARGS);
    CHECK(payload_ext_from_path("file.", ext) == STEGO_ERR_ARGS);
    CHECK(payload_ext_from_path("a.0123456789012345678901234567890123",
                                ext) == STEGO_ERR_ARGS);
}

static void test_capacity(void) {
    printf("capacity\n");
    uint8_t body[800] = {0};
    Bmp bmp;
    memset(&bmp, 0, sizeof bmp);
    bmp.data = body;
    bmp.data_size = sizeof body;

    CHECK(bmp_capacity_bytes(&bmp, STEG_LSB1) == 100);       /* 800 / 8 */
    CHECK(bmp_capacity_bytes(&bmp, STEG_LSB4) == 400);       /* 800 / 2 */
    CHECK(bmp_capacity_bytes(&bmp, STEG_LSBI) == 99);        /* 796 / 8 */
    CHECK(steg_capacity(STEG_LSB1, &bmp) == 100);
    bmp.data_size = 3;
    CHECK(bmp_capacity_bytes(&bmp, STEG_LSBI) == 0);

    /* Does not fit: the error shows the maximum capacity. */
    bmp.data_size = sizeof body;
    uint8_t big[101] = {0};
    CHECK(steg_embed(&bmp, STEG_LSB1, big, sizeof big) ==
          STEGO_ERR_CAPACITY);
    printf("  msg: %s\n", stego_error_message());
    CHECK(strstr(stego_error_message(), "100 bytes") != NULL);
    CHECK(steg_embed(&bmp, STEG_LSBI, big, 100) == STEGO_ERR_CAPACITY);
    CHECK(strstr(stego_error_message(), "99 bytes") != NULL);
}

static void test_encrypted(void) {
    printf("encrypted payload\n");
    Options opts = plain_options();
    opts.password = "oculto";
    opts.algo = CIPHER_3DES;
    opts.mode = MODE_CBC;

    const char text[] = "secreto";
    uint8_t *stream = NULL, *back = NULL;
    size_t stream_size = 0, back_size = 0;
    char ext[PAYLOAD_EXT_MAX];

    StegoStatus st = payload_build((const uint8_t *)text, strlen(text), ".txt",
                                   &opts, &stream, &stream_size);
    CHECK(st == STEGO_OK);
    hexdump("stream", stream, stream_size);
    /* cipher_size(4, BE) || cipher: 4 + 7 + 5 = 16 bytes -> 3DES pads to 24 */
    CHECK(stream_size == 4 + 24);
    CHECK(stream[0] == 0 && stream[1] == 0 && stream[2] == 0 &&
          stream[3] == 24);

    CHECK(payload_parse(stream, stream_size, &opts, &back, &back_size, ext) ==
          STEGO_OK);
    CHECK(back_size == strlen(text) && memcmp(back, text, back_size) == 0);
    CHECK(strcmp(ext, ".txt") == 0);
    free(back);
    free(stream);
}

int main(void) {
    test_roundtrips();
    test_size_byte_order();
    test_corrupt();
    test_extension();
    test_capacity();
    test_encrypted();

    printf("\n%d checks, %d failed\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
