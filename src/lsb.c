/* lsb.c - LSB1 y LSB4 */
#include "steg.h"

/* 4 bits bajos de byte */
static inline uint8_t get_low_nibble(uint8_t byte) {
    return (uint8_t)(byte & 0x0F);
}

/* devuelve byte con sus 4 bits bajos reemplazados por nibble (0..15) */
static inline uint8_t set_low_nibble(uint8_t byte, uint8_t nibble) {
    return (uint8_t)((byte & 0xF0) | nibble);
}

StegoStatus lsb1_embed(uint8_t *cover, const uint8_t *msg, size_t len) {
    for(size_t i = 0; i < len * 8; i++){
        //el bit i del secreto va al LSB del byte i de la imagen
        cover[i] = set_lsb(cover[i], msg_bit(msg, i));
    }
    return STEGO_OK;
}

StegoStatus lsb1_extract(StegReader *r, size_t len, uint8_t *out) {
    const uint8_t *stego = r->stego;
    for(size_t i = 0; i < len; i++){
        uint8_t byte = 0;
        for(size_t b=0; b < 8; b++){
            //junto el LSB de cada uno de los 8 bytes de la imagen que forman el byte de out
            byte = push_bit(byte, get_lsb(stego[r->pos++]));
        }
        out[i] = byte;
    }
    return STEGO_OK;
}

StegoStatus lsb4_embed(uint8_t *cover, const uint8_t *msg, size_t len) {
    for(size_t i = 0; i < len; i++){
        //cada byte del secreto ocupa 2 bytes de la imagen: primero el nibble alto, despues el bajo
        cover[2 * i]     = set_low_nibble(cover[2 * i],     msg[i] >> 4);
        cover[2 * i + 1] = set_low_nibble(cover[2 * i + 1], get_low_nibble(msg[i]));
    }
    return STEGO_OK;
}

StegoStatus lsb4_extract(StegReader *r, size_t len, uint8_t *out) {
    const uint8_t *stego = r->stego;
    for(size_t i = 0; i < len; i++, r->pos += 2){
        //junto los 4 bits bajos de 2 bytes consecutivos de la imagen: el primero es el nibble alto
        out[i] = (uint8_t)((get_low_nibble(stego[r->pos]) << 4) | get_low_nibble(stego[r->pos + 1]));
    }
    return STEGO_OK;
}
