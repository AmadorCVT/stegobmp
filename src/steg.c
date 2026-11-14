#include "steg.h"

#include <string.h>

/* bytes del mensaje que entran usando la imagen desde el byte pos */
static size_t capacity_from(StegMethod method, size_t len, size_t pos) {
    if (len <= pos) {
        return 0;
    }
    switch (method) {
    case STEG_LSB1:
        return (len - pos) / 8;
    case STEG_LSB4:
        return (len - pos) / 2;
    case STEG_LSBI:
        /* solo azul y verde: en [0, k) hay k/3 rojos (j % 3 == 2) */
        return ((len - pos) - (len / 3 - pos / 3)) / 8;
    }
    return 0;
}

size_t steg_capacity(StegMethod method, size_t cover_len) {
    size_t start = method == STEG_LSBI ? LSBI_PATTERNS : 0;
    return capacity_from(method, cover_len, start);
}

StegoStatus steg_embed(StegMethod method, uint8_t *cover, size_t cover_len,
                       const uint8_t *msg, size_t len) {
    size_t capacity = steg_capacity(method, cover_len);
    if (len > capacity) {
        stego_set_error("el archivo no entra en el portador: hay que ocultar "
                        "%zu bytes (incluye el tamaño, la extensión y, si hay "
                        "cifrado, el relleno) y la capacidad máxima con %s es "
                        "de %zu bytes", len, stego_method_name(method),
                        capacity);
        return STEGO_ERR_CAPACITY;
    }

    switch (method) {
    case STEG_LSB1: return lsb1_embed(cover, msg, len);
    case STEG_LSB4: return lsb4_embed(cover, msg, len);
    case STEG_LSBI: return lsbi_embed(cover, msg, len);
    }
    stego_set_error("método de esteganografiado desconocido");
    return STEGO_ERR_ARGS;
}

void steg_reader_init(StegReader *r, StegMethod method,
                      const uint8_t *stego, size_t stego_len) {
    memset(r, 0, sizeof *r);
    r->method = method;
    r->stego = stego;
    r->stego_len = stego_len;
    if (method == STEG_LSBI) {
        lsbi_reader_init(r);
    }
}

size_t steg_remaining(const StegReader *r) {
    return capacity_from(r->method, r->stego_len, r->pos);
}

StegoStatus steg_read(StegReader *r, size_t len, uint8_t *out) {
    size_t remaining = steg_remaining(r);
    if (len > remaining) {
        stego_set_error("se piden %zu bytes pero en el portador quedan %zu "
                        "con %s", len, remaining,
                        stego_method_name(r->method));
        return STEGO_ERR_CAPACITY;
    }

    switch (r->method) {
    case STEG_LSB1: return lsb1_extract(r, len, out);
    case STEG_LSB4: return lsb4_extract(r, len, out);
    case STEG_LSBI: return lsbi_extract(r, len, out);
    }
    stego_set_error("método de esteganografiado desconocido");
    return STEGO_ERR_ARGS;
}
