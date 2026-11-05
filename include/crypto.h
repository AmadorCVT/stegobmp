/*
 * crypto.h - Encryption / decryption of the payload (OpenSSL EVP).
 *
 * Fixed by the assignment:
 *   - Key and IV come from PBKDF2-HMAC-SHA256, 10000 iterations, fixed salt of
 *     8 zero bytes. One PBKDF2 output is split: first key_len bytes are the
 *     key, the next iv_len bytes are the IV (no IV for ECB).
 *   - Algorithms: AES-128/192/256 and des-ede3 (3DES, three keys).
 *   - Modes: ECB, CBC (PKCS#5/7 padding, OpenSSL default), CFB with 8 bit
 *     feedback (EVP_*_cfb8), OFB with 128 bit feedback (EVP_*_ofb).
 *     CFB8 and OFB are stream modes: no padding, output length == input length.
 */
#ifndef CRYPTO_H
#define CRYPTO_H

#include "stegobmp.h"

/*
 * Encrypts `in` (in_len bytes). On success *out is a malloc'ed buffer that the
 * caller frees, and *out_len its length.
 */
StegoStatus crypto_encrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len);

/*
 * Inverse of crypto_encrypt(). Returns STEGO_ERR_CRYPTO when the data cannot
 * be decrypted (typically a wrong password => bad padding).
 */
StegoStatus crypto_decrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len);

#endif /* CRYPTO_H */
