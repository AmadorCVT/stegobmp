/*
 * steg.h - ocultar/recuperar bytes en los bytes de una imagen.
 * no sabe nada del bmp (recibe el cuerpo y su largo) ni de tamaños ni
 * cifrado, eso lo arma payload.c.
 * los bits van en orden, primero el mas significativo de cada byte.
 */
#ifndef STEG_H
#define STEG_H

#include "stegobmp.h"

/* LSBI: cantidad de grupos, y de bytes al principio que guardan si se invierten */
#define LSBI_PATTERNS 4

/* Cuántos bytes entran en una imagen de cover_len bytes: LSB1 cover_len/8,
 * LSB4 cover_len/2, LSBI (bytes azules y verdes despues de los 4 primeros)/8. */
size_t steg_capacity(StegMethod method, size_t cover_len);

/* oculta msg en cover (in place). STEGO_ERR_CAPACITY si no entra */
StegoStatus steg_embed(StegMethod method, uint8_t *cover, size_t cover_len,
                       const uint8_t *msg, size_t len);

/*
 * Lector secuencial: cada steg_read() sigue donde terminó el anterior, asi
 * payload.c puede leer primero el tamaño y despues el resto sin releer.
 * El estado propio de cada método (LSBI: que grupos se invierten) lo
 * arma steg_reader_init(), afuera no hace falta saberlo.
 */
typedef struct {
    StegMethod     method;
    const uint8_t *stego;
    size_t         stego_len;
    size_t         pos;                     /* proximo byte de la imagen */
    uint8_t        inverted[LSBI_PATTERNS]; /* solo LSBI */
} StegReader;

void steg_reader_init(StegReader *r, StegMethod method,
                      const uint8_t *stego, size_t stego_len);

/* bytes del mensaje que quedan por leer */
size_t steg_remaining(const StegReader *r);

/* lee los proximos len bytes en out. STEGO_ERR_CAPACITY si no hay tantos */
StegoStatus steg_read(StegReader *r, size_t len, uint8_t *out);

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

#endif /* STEG_H */
