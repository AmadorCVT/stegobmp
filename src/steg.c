#include "steg.h"

size_t steg_capacity(StegMethod method, const Bmp *bmp) {
    return bmp_capacity_bytes(bmp, method);
}

StegoStatus steg_embed(Bmp *bmp, StegMethod method,
                       const uint8_t *data, size_t len) {
    size_t capacity = steg_capacity(method, bmp);
    if (len > capacity) {
        stego_set_error("el archivo no entra en el portador: hay que ocultar "
                        "%zu bytes (incluye el tamaño, la extensión y, si hay "
                        "cifrado, el relleno) y la capacidad máxima con %s es "
                        "de %zu bytes", len, stego_method_name(method),
                        capacity);
        return STEGO_ERR_CAPACITY;
    }

    switch (method) {
    case STEG_LSB1: return lsb1_embed(bmp, data, len);
    case STEG_LSB4: return lsb4_embed(bmp, data, len);
    case STEG_LSBI: return lsbi_embed(bmp, data, len);
    }
    stego_set_error("método de esteganografiado desconocido");
    return STEGO_ERR_ARGS;
}

StegoStatus steg_extract(const Bmp *bmp, StegMethod method,
                         size_t len, uint8_t *out) {
    size_t capacity = steg_capacity(method, bmp);
    if (len > capacity) {
        stego_set_error("se piden %zu bytes pero la capacidad máxima del "
                        "portador con %s es de %zu bytes", len,
                        stego_method_name(method), capacity);
        return STEGO_ERR_CAPACITY;
    }

    switch (method) {
    case STEG_LSB1: return lsb1_extract(bmp, len, out);
    case STEG_LSB4: return lsb4_extract(bmp, len, out);
    case STEG_LSBI: return lsbi_extract(bmp, len, out);
    }
    stego_set_error("método de esteganografiado desconocido");
    return STEGO_ERR_ARGS;
}
