/*
 * crypto.c - Encryption with OpenSSL. See the contract in include/crypto.h.
 *
 * Placeholder bodies: the owner of this file replaces them (and adds
 * -lcrypto to LDLIBS in the Makefile). Do not change the signatures.
 */
#include "crypto.h"

StegoStatus crypto_encrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {
    (void)algo; (void)mode; (void)password;
    (void)in; (void)in_len; (void)out; (void)out_len;
    return stego_not_implemented("cifrado");
}

StegoStatus crypto_decrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {
    (void)algo; (void)mode; (void)password;
    (void)in; (void)in_len; (void)out; (void)out_len;
    return stego_not_implemented("descifrado");
}
