/*
 * steg.h - ocultar/recuperar bytes en el cuerpo del bmp.
 * aca no se sabe nada de tamaños ni cifrado, eso lo arma payload.c.
 * los bits van en orden, primero el mas significativo de cada byte.
 */
#ifndef STEG_H
#define STEG_H

#include "bmp.h"
#include "stegobmp.h"

/* Cuántos bytes entran: LSB1 data_size/8, LSB4 data_size/2,
 * LSBI (data_size-4)/8. */
size_t steg_capacity(StegMethod method, const Bmp *bmp);

/* oculta data en bmp->data. STEGO_ERR_CAPACITY si no entra */
StegoStatus steg_embed(Bmp *bmp, StegMethod method,
                       const uint8_t *data, size_t len);

/* recupera los primeros len bytes en out, se puede llamar varias veces */
StegoStatus steg_extract(const Bmp *bmp, StegMethod method,
                         size_t len, uint8_t *out);

/* steg.c ya chequea que len <= capacidad antes de llamar a estas */

/* lsb.c - LSB1 usa 1 bit por byte de la imagen, LSB4 usa 4 (nibble alto primero) */
StegoStatus lsb1_embed(Bmp *bmp, const uint8_t *data, size_t len);
StegoStatus lsb1_extract(const Bmp *bmp, size_t len, uint8_t *out);
StegoStatus lsb4_embed(Bmp *bmp, const uint8_t *data, size_t len);
StegoStatus lsb4_extract(const Bmp *bmp, size_t len, uint8_t *out);

/* lsbi.c - los primeros 4 bytes guardan los bits del patron (con LSB1),
 * despues va 1 bit por byte invirtiendo segun el patron */
StegoStatus lsbi_embed(Bmp *bmp, const uint8_t *data, size_t len);
StegoStatus lsbi_extract(const Bmp *bmp, size_t len, uint8_t *out);

#endif /* STEG_H */
