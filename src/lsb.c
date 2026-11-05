/*
 * lsb.c - LSB1 and LSB4. See the contract in include/steg.h.
 *
 * Placeholder bodies: the owner of this file replaces them. Do not change the
 * signatures.
 */
#include "steg.h"

StegoStatus lsb1_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    (void)bmp; (void)data; (void)len;
    return stego_not_implemented("LSB1 (ocultar)");
}

StegoStatus lsb1_extract(const Bmp *bmp, size_t len, uint8_t *out) {
    (void)bmp; (void)len; (void)out;
    return stego_not_implemented("LSB1 (extraer)");
}

StegoStatus lsb4_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    (void)bmp; (void)data; (void)len;
    return stego_not_implemented("LSB4 (ocultar)");
}

StegoStatus lsb4_extract(const Bmp *bmp, size_t len, uint8_t *out) {
    (void)bmp; (void)len; (void)out;
    return stego_not_implemented("LSB4 (extraer)");
}
