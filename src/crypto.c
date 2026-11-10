/*
 * crypto.c - Encryption with OpenSSL. See the contract in include/crypto.h.
 *
 * Pending: the owner of this file implements these functions (and adds
 * -lcrypto to LDLIBS in the Makefile). Do not change the signatures.
 */

#include "crypto.h"
#include "stegobmp.h"
#include <openssl/evp.h>
#include <string.h>

#define PBKDF2_SALT  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define PBKDF2_ITERATIONS 10000

static const EVP_CIPHER *(*const cipher_map[4][4])(void) = {
    {EVP_aes_128_ecb,   EVP_aes_128_cfb8,   EVP_aes_128_ofb,    EVP_aes_128_cbc},
    {EVP_aes_192_ecb,   EVP_aes_192_cfb8,   EVP_aes_192_ofb,    EVP_aes_192_cbc },
    {EVP_aes_256_ecb,   EVP_aes_256_cfb8,   EVP_aes_256_ofb,    EVP_aes_256_cbc},
    {EVP_des_ede3_ecb,  EVP_des_ede3_cfb8,  EVP_des_ede3_ofb,   EVP_des_ede3_cbc},
};

/*
 * Derives key and IV from password using PBKDF2-HMAC-SHA256 (PBKDF2_ITERATIONS iterations,
 * PBKDF2_SALT salt).
 * Copies key to key_out, and IV to iv_out only if required by the cipher (non-ECB).
 * Returns:
 *   - STEGO_OK on success.
 *   - STEGO_ERR_CRYPTO if PBKDF2 derivation fails.
 */
static StegoStatus get_key_iv(const EVP_CIPHER *cipher, const char *password, uint8_t *key_out, uint8_t *iv_out);

StegoStatus crypto_encrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {
    
    if (password == NULL || in == NULL || out == NULL || out_len == NULL) {
        stego_set_error("Invalid parameters, aborting encryption");
        return STEGO_ERR_ARGS;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new(); 
    if (ctx == NULL) {
        stego_set_error("Failed to create encryption context");
        return STEGO_ERR_CRYPTO;
    }

    const EVP_CIPHER *cipher = cipher_map[algo][mode]();
    uint8_t key_out[EVP_MAX_KEY_LENGTH], iv_out[EVP_MAX_IV_LENGTH];
    StegoStatus err = get_key_iv(cipher, password, key_out, iv_out);
    if (err != STEGO_OK) {
        EVP_CIPHER_CTX_free(ctx);
        return err;
    }



    
    
    


    
    EVP_CIPHER_CTX_free(ctx);
}

StegoStatus crypto_decrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {

    if (password == NULL || in == NULL || out == NULL || out_len == NULL) {
        stego_set_error("Invalid parameters, aborting encryption");
        return STEGO_ERR_ARGS;                
    }
}



static StegoStatus get_key_iv(const EVP_CIPHER *cipher, const char *password, uint8_t *key_out, uint8_t *iv_out){
   
    uint8_t salt[8] = PBKDF2_SALT;
    int key_length = EVP_CIPHER_key_length(cipher);
    int iv_length = EVP_CIPHER_iv_length(cipher);
    int total_length = key_length + iv_length;
    uint8_t key_iv_buf[EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH];

    // int PKCS5_PBKDF2_HMAC(const char *pass, int passlen, const unsigned char *salt, int saltlen, int iter, const EVP_MD *digest, int keylen, unsigned char *out);
    // Generate key+iv with password, salt = 0x00 (8), 10000 iterations, sha256 algorithm.
    if (!PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, sizeof(salt), PBKDF2_ITERATIONS, EVP_sha256(), total_length, key_iv_buf)) {
        stego_set_error("failed to derive key with PBKDF2");
        return STEGO_ERR_CRYPTO; 
    }

    memcpy(key_out, key_iv_buf, key_length);
    if (iv_length > 0 && iv_out != NULL) {
        memcpy(iv_out, key_iv_buf + key_length, iv_length);
    }
    return STEGO_OK;
} 