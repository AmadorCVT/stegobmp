/*
 * steg_impl.h - interno de la capa steg: lo usan solo steg.c, lsb.c y lsbi.c.
 * payload.c y main.c tienen que incluir steg.h, no este.
 */
#ifndef STEG_IMPL_H
#define STEG_IMPL_H

#include "steg.h"

/* ayudas para trabajar bit a bit, las usan lsb.c y lsbi.c */

/* bit i del mensaje, contando desde el mas significativo de data[0] */
static inline uint8_t msg_bit(const uint8_t *data, size_t i) {
    return (uint8_t)((data[i / 8] >> (7 - i % 8)) & 1);
}

/* LSB de byte (0 o 1) */
static inline uint8_t get_lsb(uint8_t byte) {
    return (uint8_t)(byte & 0x01);
}

/* devuelve byte con su LSB reemplazado por bit (0 o 1) */
static inline uint8_t set_lsb(uint8_t byte, uint8_t bit) {
    return (uint8_t)((byte & 0xFE) | bit);
}

/* agrega bit a la derecha de acc: para ir armando bytes al extraer */
static inline uint8_t push_bit(uint8_t acc, uint8_t bit) {
    return (uint8_t)((acc << 1) | bit);
}

/*
 * Estas reciben los bytes de la imagen (como el paper, cover C y stego S).
 * El embed modifica cover in place, que queda como stego. Los extract leen
 * desde r->pos y lo dejan apuntando al byte siguiente al ultimo que usaron.
 * steg.c ya chequea que len <= capacidad antes de llamar a estas.
 */

/* lsb.c - LSB1 usa 1 bit por byte de la imagen, LSB4 usa 4 (nibble alto primero) */
StegoStatus lsb1_embed(uint8_t *cover, const uint8_t *msg, size_t len);
StegoStatus lsb1_extract(StegReader *r, size_t len, uint8_t *out);
StegoStatus lsb4_embed(uint8_t *cover, const uint8_t *msg, size_t len);
StegoStatus lsb4_extract(StegReader *r, size_t len, uint8_t *out);

/* lsbi.c - los primeros 4 bytes guardan los bits del patron (con LSB1),
 * despues va 1 bit por byte azul o verde (saltea los rojos) invirtiendo
 * segun el patron. lsbi_reader_init lee esos 4 bytes y deja pos despues. */
StegoStatus lsbi_embed(uint8_t *cover, const uint8_t *msg, size_t len);
void        lsbi_reader_init(StegReader *r);
StegoStatus lsbi_extract(StegReader *r, size_t len, uint8_t *out);

#endif /* STEG_IMPL_H */
