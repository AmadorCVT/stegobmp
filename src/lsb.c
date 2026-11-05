/*
 * lsb.c - LSB1 and LSB4. See the contract in include/steg.h.
 *
 * Pending: the owner of this file implements these functions. Do not change
 * the signatures.
 */
#include "steg.h"


StegoStatus lsb1_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    for(size_t i= 0; i < len * 8; i++){
        uint8_t bit = (data[i / 8] >> (7 - i % 8)) & 1;//recojo el bit a ocultar
        bmp->data[i] = ((bmp->data[i] & 0xFE) | bit) ; //tomo el byte la imagen, borro el ultimo bit y le inserto el bit del secreto
    }
    return STEGO_OK;
}

StegoStatus lsb1_extract(const Bmp *bmp, size_t len, uint8_t *out) {
}

StegoStatus lsb4_embed(Bmp *bmp, const uint8_t *data, size_t len) {
}

StegoStatus lsb4_extract(const Bmp *bmp, size_t len, uint8_t *out) {
}
