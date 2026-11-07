/*
 * lsb.c - LSB1 and LSB4. See the contract in include/steg.h.
 *
 * Pending: the owner of this file implements these functions. Do not change
 * the signatures.
 */
#include "steg.h"


StegoStatus lsb1_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    for(size_t i = 0; i < len * 8; i++){
        uint8_t bit = (data[i / 8] >> (7 - i % 8)) & 1;//recojo el bit a ocultar
        bmp->data[i] = (bmp->data[i] & 0xFE) | bit; //tomo el byte de la imagen, borro el ultimo bit y le inserto el bit del secreto
    }
    return STEGO_OK;
}

StegoStatus lsb1_extract(const Bmp *bmp, size_t len, uint8_t *out) {
    for(size_t i = 0; i < len; i++){
        uint8_t byte = 0;
        for(size_t b=0; b < 8; b++){
            byte = (uint8_t) ((byte << 1) | (bmp->data[i * 8 + b] & 1)); //tomo el byte, lo desplazo uno a la izq y tomo el ultimo bit de cada uno de los 8 bytes de la imagen que formaran el byte de out
        }
        out[i] = byte;
    }
    return STEGO_OK;
}

StegoStatus lsb4_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    for(size_t i = 0; i < len; i++){
        //cada byte del secreto ocupa 2 bytes de la imagen: primero el nibble alto, despues el bajo
        bmp->data[2 * i]     = (uint8_t) ((bmp->data[2 * i]     & 0xF0) | (data[i] >> 4));
        bmp->data[2 * i + 1] = (uint8_t) ((bmp->data[2 * i + 1] & 0xF0) | (data[i] & 0x0F));
    }
    return STEGO_OK;
}

StegoStatus lsb4_extract(const Bmp *bmp, size_t len, uint8_t *out) {
    for(size_t i = 0; i < len; i++){
        //junto los 4 bits bajos de 2 bytes consecutivos de la imagen: el primero es el nibble alto
        out[i] = (uint8_t) (((bmp->data[2 * i] & 0x0F) << 4) | (bmp->data[2 * i + 1] & 0x0F));
    }
    return STEGO_OK;
}
