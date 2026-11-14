/* lsb.c - LSB1 y LSB4 */
#include "steg.h"


StegoStatus lsb1_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    for(size_t i = 0; i < len * 8; i++){
        //el bit i del secreto va al LSB del byte i de la imagen
        bmp->data[i] = set_lsb(bmp->data[i], msg_bit(data, i));
    }
    return STEGO_OK;
}

StegoStatus lsb1_extract(const Bmp *bmp, size_t len, uint8_t *out) {
    for(size_t i = 0; i < len; i++){
        uint8_t byte = 0;
        for(size_t b=0; b < 8; b++){
            //junto el LSB de cada uno de los 8 bytes de la imagen que forman el byte de out
            byte = push_bit(byte, get_lsb(bmp->data[i * 8 + b]));
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
