/* payload.c - arma y lee lo que se oculta en el bmp (formato en payload.h) */
#include "payload.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto.h"
#include "steg.h"

#define SIZE_FIELD 4

static void put_u32_be(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static uint32_t get_u32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

StegoStatus payload_ext_from_path(const char *path, char ext[PAYLOAD_EXT_MAX]) {
    /* buscamos el ultimo '.' del nombre, o sea despues de la ultima barra */
    const char *name = path;
    for (const char *p = path; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            name = p + 1;
        }
    }
    const char *dot = strrchr(name, '.');
    if (dot == NULL || dot == name || dot[1] == '\0') {
        stego_set_error("el archivo '%s' no tiene extensión: es necesaria "
                        "para poder recuperarlo (por ejemplo .txt)", path);
        return STEGO_ERR_ARGS;
    }

    size_t len = strlen(dot);
    if (len + 1 > PAYLOAD_EXT_MAX) {
        stego_set_error("la extensión '%s' es demasiado larga (máximo %d "
                        "caracteres)", dot, PAYLOAD_EXT_MAX - 1);
        return STEGO_ERR_ARGS;
    }
    memcpy(ext, dot, len + 1);
    return STEGO_OK;
}

/* tamaño (4, big endian) || archivo || ext || '\0' */
static StegoStatus build_plain(const uint8_t *file, size_t file_size,
                               const char *ext,
                               uint8_t **out, size_t *out_size) {
    size_t ext_len = strlen(ext);
    if (ext_len < 2 || ext[0] != '.' || ext_len + 1 > PAYLOAD_EXT_MAX) {
        stego_set_error("extensión inválida '%s': debe comenzar con '.' y "
                        "tener al menos un carácter más", ext);
        return STEGO_ERR_ARGS;
    }
    if (file_size > UINT32_MAX - SIZE_FIELD - PAYLOAD_EXT_MAX) {
        stego_set_error("el archivo es demasiado grande (%zu bytes)",
                        file_size);
        return STEGO_ERR_ARGS;
    }

    size_t total = SIZE_FIELD + file_size + ext_len + 1;
    uint8_t *buf = malloc(total);
    if (buf == NULL) {
        stego_set_error("sin memoria para armar el payload");
        return STEGO_ERR_MEMORY;
    }
    put_u32_be(buf, (uint32_t)file_size);
    if (file_size > 0) {
        memcpy(buf + SIZE_FIELD, file, file_size);
    }
    memcpy(buf + SIZE_FIELD + file_size, ext, ext_len + 1);

    *out = buf;
    *out_size = total;
    return STEGO_OK;
}

StegoStatus payload_build(const uint8_t *file, size_t file_size,
                          const char ext[PAYLOAD_EXT_MAX],
                          const Options *opts,
                          uint8_t **stream, size_t *stream_size) {
    uint8_t *plain = NULL;
    size_t plain_size = 0;
    StegoStatus st = build_plain(file, file_size, ext, &plain, &plain_size);
    if (st != STEGO_OK) {
        return st;
    }
    if (!options_use_crypto(opts)) {
        *stream = plain;
        *stream_size = plain_size;
        return STEGO_OK;
    }

    /* Tamaño del cifrado (4, BE) || cifrado */
    uint8_t *cipher = NULL;
    size_t cipher_size = 0;
    st = crypto_encrypt(opts->algo, opts->mode, opts->password,
                        plain, plain_size, &cipher, &cipher_size);
    free(plain);
    if (st != STEGO_OK) {
        return st;
    }
    if (cipher_size > UINT32_MAX) {
        free(cipher);
        stego_set_error("el archivo cifrado es demasiado grande");
        return STEGO_ERR_ARGS;
    }

    uint8_t *buf = malloc(SIZE_FIELD + cipher_size);
    if (buf == NULL) {
        free(cipher);
        stego_set_error("sin memoria para armar el payload");
        return STEGO_ERR_MEMORY;
    }
    put_u32_be(buf, (uint32_t)cipher_size);
    memcpy(buf + SIZE_FIELD, cipher, cipher_size);
    free(cipher);

    *stream = buf;
    *stream_size = SIZE_FIELD + cipher_size;
    return STEGO_OK;
}

#define CORRUPT_HINT "(¿método -steg, password, -a o -m incorrectos?)"

static bool valid_ext_char(unsigned char c) {
    return c > 0x20 && c < 0x7F && c != '/' && c != '\\';
}

/* e[0..n) es la extension sin el '\0': '.' y al menos un caracter valido */
static StegoStatus check_ext(const uint8_t *e, size_t n) {
    bool ok = n >= 2 && e[0] == '.';
    for (size_t i = 0; ok && i < n; i++) {
        ok = valid_ext_char(e[i]);
    }
    if (!ok) {
        stego_set_error("datos corruptos: la extensión recuperada no es "
                        "válida %s", CORRUPT_HINT);
        return STEGO_ERR_CORRUPT;
    }
    return STEGO_OK;
}

/* lee tamaño || archivo || ext || '\0', lo que sobra al final se ignora */
static StegoStatus parse_plain(const uint8_t *buf, size_t buf_size,
                               uint8_t **file, size_t *file_size,
                               char ext[PAYLOAD_EXT_MAX]) {
    if (buf_size < SIZE_FIELD) {
        stego_set_error("datos corruptos: no hay lugar para el tamaño %s",
                        CORRUPT_HINT);
        return STEGO_ERR_CORRUPT;
    }
    size_t size = get_u32_be(buf);
    size_t avail = buf_size - SIZE_FIELD;
    if (size > avail) {
        stego_set_error("datos corruptos: el tamaño declarado (%zu bytes) es "
                        "mayor que los datos disponibles (%zu bytes) %s",
                        size, avail, CORRUPT_HINT);
        return STEGO_ERR_CORRUPT;
    }

    /* La extensión va justo después del archivo: '.' + caracteres + '\0'. */
    const uint8_t *e = buf + SIZE_FIELD + size;
    size_t room = avail - size;
    size_t n = 0;
    while (n < room && n < PAYLOAD_EXT_MAX && e[n] != '\0') {
        n++;
    }
    if (n >= room || n >= PAYLOAD_EXT_MAX) {
        stego_set_error("datos corruptos: la extensión no termina en '\\0' "
                        "%s", CORRUPT_HINT);
        return STEGO_ERR_CORRUPT;
    }
    StegoStatus st = check_ext(e, n);
    if (st != STEGO_OK) {
        return st;
    }

    uint8_t *out = malloc(size > 0 ? size : 1);
    if (out == NULL) {
        stego_set_error("sin memoria para recuperar el archivo");
        return STEGO_ERR_MEMORY;
    }
    if (size > 0) {
        memcpy(out, buf + SIZE_FIELD, size);
    }
    memcpy(ext, e, n + 1);
    *file = out;
    *file_size = size;
    return STEGO_OK;
}

/* descifra cipher y lee lo de adentro (tamaño || archivo || ext || '\0') */
static StegoStatus decrypt_and_parse(const uint8_t *cipher, size_t cipher_size,
                                     const Options *opts,
                                     uint8_t **file, size_t *file_size,
                                     char ext[PAYLOAD_EXT_MAX]) {
    uint8_t *plain = NULL;
    size_t plain_size = 0;
    StegoStatus st = crypto_decrypt(opts->algo, opts->mode, opts->password,
                                    cipher, cipher_size,
                                    &plain, &plain_size);
    if (st != STEGO_OK) {
        if (st == STEGO_ERR_CRYPTO) {
            /* le agregamos la pista al mensaje que dejo crypto.c */
            char detail[256];
            snprintf(detail, sizeof detail, "%s", stego_error_message());
            stego_set_error("no se pudo descifrar los datos ocultos (%s) %s",
                            detail, CORRUPT_HINT);
        }
        return st;
    }
    st = parse_plain(plain, plain_size, file, file_size, ext);
    free(plain);
    return st;
}

/* sin cifrar, la extension va despues del archivo: se lee de a un byte hasta el '\0' */
static StegoStatus read_ext(StegReader *r, char ext[PAYLOAD_EXT_MAX]) {
    uint8_t e[PAYLOAD_EXT_MAX];
    for (size_t n = 0; n < PAYLOAD_EXT_MAX && steg_remaining(r) > 0; n++) {
        StegoStatus st = steg_read(r, 1, &e[n]);
        if (st != STEGO_OK) {
            return st;
        }
        if (e[n] == '\0') {
            if ((st = check_ext(e, n)) != STEGO_OK) {
                return st;
            }
            memcpy(ext, e, n + 1);
            return STEGO_OK;
        }
    }
    stego_set_error("datos corruptos: la extensión no termina en '\\0' %s",
                    CORRUPT_HINT);
    return STEGO_ERR_CORRUPT;
}

StegoStatus payload_extract(const uint8_t *stego, size_t stego_len,
                            const Options *opts,
                            uint8_t **file, size_t *file_size,
                            char ext[PAYLOAD_EXT_MAX]) {
    StegReader r;
    steg_reader_init(&r, opts->method, stego, stego_len);
    if (steg_remaining(&r) < SIZE_FIELD) {
        stego_set_error("el portador es demasiado chico para contener datos "
                        "ocultos con %s (capacidad: %zu bytes)",
                        stego_method_name(opts->method), steg_remaining(&r));
        return STEGO_ERR_CORRUPT;
    }

    /* primero el tamaño, para saber cuanto hay que leer y reservar */
    uint8_t header[SIZE_FIELD];
    StegoStatus st = steg_read(&r, SIZE_FIELD, header);
    if (st != STEGO_OK) {
        return st;
    }
    size_t size = get_u32_be(header);
    if (size > steg_remaining(&r)) {
        stego_set_error("datos corruptos: el tamaño declarado (%zu bytes) es "
                        "mayor que la capacidad disponible (%zu bytes) %s",
                        size, steg_remaining(&r), CORRUPT_HINT);
        return STEGO_ERR_CORRUPT;
    }

    /* despues el archivo (o el cifrado), sigue donde quedo el lector */
    uint8_t *buf = malloc(size > 0 ? size : 1);
    if (buf == NULL) {
        stego_set_error("sin memoria para leer los datos ocultos");
        return STEGO_ERR_MEMORY;
    }
    st = steg_read(&r, size, buf);

    if (st == STEGO_OK && options_use_crypto(opts)) {
        st = decrypt_and_parse(buf, size, opts, file, file_size, ext);
        free(buf);
        return st;
    }
    if (st == STEGO_OK) {
        st = read_ext(&r, ext);
    }
    if (st != STEGO_OK) {
        free(buf);
        return st;
    }
    *file = buf;
    *file_size = size;
    return STEGO_OK;
}
