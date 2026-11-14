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
 * Los campos son internos (los usan steg.c, lsb.c y lsbi.c): desde afuera
 * solo se declara en el stack y se usa con las funciones de abajo.
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

#endif /* STEG_H */
