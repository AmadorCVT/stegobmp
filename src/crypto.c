/*
 * crypto.c - Encryption with OpenSSL. See the contract in include/crypto.h.
 *
 * Pending: the owner of this file implements these functions (and adds
 * -lcrypto to LDLIBS in the Makefile). Do not change the signatures.
 */
#include "crypto.h"

StegoStatus crypto_encrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {
}

StegoStatus crypto_decrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {
}
