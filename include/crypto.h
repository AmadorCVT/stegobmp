/*
 * crypto.h - cifrado del payload con OpenSSL.
 * key e iv salen de PBKDF2-HMAC-SHA256 (10000 iteraciones, salt de 8 ceros).
 * cfb es cfb8 y ofb es de 128 bits, como pide la consigna.
 */
#ifndef CRYPTO_H
#define CRYPTO_H

#include "stegobmp.h"

/* cifra in, *out hay que liberarlo */
StegoStatus crypto_encrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len);

/* Descifra. Devuelve STEGO_ERR_CRYPTO si falla (casi siempre es la password). */
StegoStatus crypto_decrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len);

#endif /* CRYPTO_H */
