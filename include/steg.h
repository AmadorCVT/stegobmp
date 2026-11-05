/*
 * steg.h - Bit-level hiding inside the BMP body (Bmp.data).
 *
 * This layer knows nothing about file sizes, extensions or encryption: it
 * hides / recovers a raw sequence of bytes ("stream"). The format of that
 * stream is built by payload.c.
 *
 * Bit order of the stream (all methods): bytes in order, most significant bit
 * of each byte first.
 */
#ifndef STEG_H
#define STEG_H

#include "bmp.h"
#include "stegobmp.h"

/* ------------------------------------------------------------------------ */
/* Dispatcher (src/steg.c)                                                  */
/* ------------------------------------------------------------------------ */

/*
 * Maximum number of stream bytes that `method` can hide in `bmp`:
 *   LSB1: data_size / 8        (1 stream bit  per body byte)
 *   LSB4: data_size / 2        (4 stream bits per body byte)
 *   LSBI: (data_size - 4) / 8  (the first 4 body bytes hold the pattern bits)
 */
size_t steg_capacity(StegMethod method, const Bmp *bmp);

/*
 * Hides `len` bytes of `data` in bmp->data (modified in place) using `method`.
 * Returns STEGO_ERR_CAPACITY (message includes steg_capacity()) if it does not
 * fit; otherwise the capacity check is already done when lsb*_embed runs.
 */
StegoStatus steg_embed(Bmp *bmp, StegMethod method,
                       const uint8_t *data, size_t len);

/*
 * Recovers the first `len` bytes of the stream into `out` (caller provides
 * `len` bytes). Returns STEGO_ERR_CAPACITY if len > steg_capacity().
 * It can be called several times with different lengths on the same Bmp
 * (payload.c reads the 4 byte size first, then the rest).
 */
StegoStatus steg_extract(const Bmp *bmp, StegMethod method,
                         size_t len, uint8_t *out);

/* ------------------------------------------------------------------------ */
/* Per-method implementations. steg.c guarantees len <= capacity.           */
/* ------------------------------------------------------------------------ */

/* LSB1 / LSB4 - src/lsb.c.
 * LSB1: stream bit k replaces the least significant bit of body byte k.
 * LSB4: stream nibble k (high nibble of each byte first) replaces the 4 least
 *       significant bits of body byte k. */
StegoStatus lsb1_embed(Bmp *bmp, const uint8_t *data, size_t len);
StegoStatus lsb1_extract(const Bmp *bmp, size_t len, uint8_t *out);
StegoStatus lsb4_embed(Bmp *bmp, const uint8_t *data, size_t len);
StegoStatus lsb4_extract(const Bmp *bmp, size_t len, uint8_t *out);

/* LSBI (Majeed & Sulaiman) - src/lsbi.c.
 * Body bytes 0..3 store the 4 pattern bits with plain LSB1; the stream starts
 * at body byte 4, one bit per byte, inverting the LSB according to the
 * pattern table. */
StegoStatus lsbi_embed(Bmp *bmp, const uint8_t *data, size_t len);
StegoStatus lsbi_extract(const Bmp *bmp, size_t len, uint8_t *out);

#endif /* STEG_H */
