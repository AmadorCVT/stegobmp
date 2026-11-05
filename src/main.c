#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"
#include "cli.h"
#include "payload.h"
#include "steg.h"
#include "stegobmp.h"

#define EXIT_OK      0
#define EXIT_RUNTIME 1
#define EXIT_USAGE   2

/* Reads a whole file. On success *buf is malloc'ed (never NULL). */
static StegoStatus read_file(const char *path, uint8_t **buf, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        stego_set_error("no se pudo abrir '%s' para lectura", path);
        return STEGO_ERR_IO;
    }

    long len = -1;
    if (fseek(f, 0, SEEK_END) == 0) {
        len = ftell(f);
    }
    if (len < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        stego_set_error("no se pudo determinar el tamaño de '%s'", path);
        return STEGO_ERR_IO;
    }

    uint8_t *data = malloc(len > 0 ? (size_t)len : 1);
    if (data == NULL) {
        fclose(f);
        stego_set_error("sin memoria para leer '%s'", path);
        return STEGO_ERR_MEMORY;
    }
    if (fread(data, 1, (size_t)len, f) != (size_t)len) {
        fclose(f);
        free(data);
        stego_set_error("error leyendo '%s'", path);
        return STEGO_ERR_IO;
    }
    fclose(f);

    *buf = data;
    *size = (size_t)len;
    return STEGO_OK;
}

static StegoStatus write_file(const char *path, const uint8_t *buf,
                              size_t size) {
    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        stego_set_error("no se pudo abrir '%s' para escritura", path);
        return STEGO_ERR_IO;
    }
    bool ok = fwrite(buf, 1, size, f) == size;
    if (fclose(f) != 0) {
        ok = false;
    }
    if (!ok) {
        stego_set_error("error escribiendo '%s'", path);
        return STEGO_ERR_IO;
    }
    return STEGO_OK;
}

static StegoStatus run_embed(const Options *opts) {
    Bmp bmp;
    uint8_t *file = NULL, *stream = NULL;
    size_t file_size = 0, stream_size = 0;
    char ext[PAYLOAD_EXT_MAX];

    /* The carrier is validated first: an unusable BMP is the clearest error. */
    StegoStatus st = bmp_load(opts->carrier_path, &bmp);
    if (st != STEGO_OK) {
        return st;
    }
    if ((st = read_file(opts->in_path, &file, &file_size)) != STEGO_OK ||
        (st = payload_ext_from_path(opts->in_path, ext)) != STEGO_OK) {
        goto done;
    }

    st = payload_build(file, file_size, ext, opts, &stream, &stream_size);
    if (st == STEGO_OK) {
        st = steg_embed(&bmp, opts->method, stream, stream_size);
    }
    if (st == STEGO_OK) {
        st = bmp_save(opts->out_path, &bmp);
    }

done:
    free(file);
    free(stream);
    bmp_free(&bmp);
    return st;
}

static StegoStatus run_extract(const Options *opts) {
    Bmp bmp;
    uint8_t *file = NULL;
    size_t file_size = 0;
    char ext[PAYLOAD_EXT_MAX];
    char *out_path = NULL;

    StegoStatus st = bmp_load(opts->carrier_path, &bmp);
    if (st != STEGO_OK) {
        return st;
    }
    st = payload_extract(&bmp, opts, &file, &file_size, ext);
    if (st != STEGO_OK) {
        goto done;
    }

    /* -out has no extension: the recovered one is appended. */
    out_path = malloc(strlen(opts->out_path) + strlen(ext) + 1);
    if (out_path == NULL) {
        stego_set_error("sin memoria");
        st = STEGO_ERR_MEMORY;
        goto done;
    }
    strcpy(out_path, opts->out_path);
    strcat(out_path, ext);
    st = write_file(out_path, file, file_size);

done:
    free(out_path);
    free(file);
    bmp_free(&bmp);
    return st;
}

int main(int argc, char *argv[]) {
    Options opts;

    if (cli_parse(argc, argv, &opts) != STEGO_OK) {
        fprintf(stderr, "stegobmp: error: %s\n\n", stego_error_message());
        cli_print_usage(stderr);
        return EXIT_USAGE;
    }
    if (opts.show_help) {
        cli_print_usage(stdout);
        return EXIT_OK;
    }

    StegoStatus st = opts.op == OP_EMBED ? run_embed(&opts) : run_extract(&opts);
    if (st != STEGO_OK) {
        fprintf(stderr, "stegobmp: error: %s\n", stego_error_message());
        return EXIT_RUNTIME;
    }
    return EXIT_OK;
}
