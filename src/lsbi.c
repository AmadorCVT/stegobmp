/*
 * lsbi.c - LSB Improved (Majeed & Sulaiman). See the contract in
 * include/steg.h.
 *
 * Placeholder bodies: the owner of this file replaces them. Do not change the
 * signatures.
 */
#include "steg.h"

StegoStatus lsbi_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    (void)bmp; (void)data; (void)len;
    return stego_not_implemented("LSBI (ocultar)");
}

StegoStatus lsbi_extract(const Bmp *bmp, size_t len, uint8_t *out) {
    (void)bmp; (void)len; (void)out;
    return stego_not_implemented("LSBI (extraer)");
}
