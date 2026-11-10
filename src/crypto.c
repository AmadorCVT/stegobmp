/*
 * crypto.c - Encryption with OpenSSL. See the contract in include/crypto.h.
 *
 * Pending: the owner of this file implements these functions (and adds
 * -lcrypto to LDLIBS in the Makefile). Do not change the signatures.
 */

#include "crypto.h"
#include "stegobmp.h"
#include <openssl/evp.h>
#include <stdlib.h>
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
    
    if (password == NULL || in == NULL || out == NULL || out_len == NULL || algo < 0 || mode < 0 || algo > 3 || mode > 3) {
        stego_set_error("Parámetros inválidos al intentar cifrar");
        return STEGO_ERR_ARGS;
    }

    const EVP_CIPHER *cipher = cipher_map[algo][mode]();

    uint8_t key[EVP_MAX_KEY_LENGTH], iv_raw[EVP_MAX_IV_LENGTH];
    StegoStatus err = get_key_iv(cipher, password, key, iv_raw);
    if (err != STEGO_OK) {
        return err;
    }
    const uint8_t *iv = (EVP_CIPHER_iv_length(cipher) > 0) ? iv_raw : NULL;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new(); 
    if (ctx == NULL) {
        stego_set_error("No se pudo crear el contexto de cifrado");
        return STEGO_ERR_MEMORY;
    }

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv) != 1) {
        stego_set_error("Falló la inicialización del cifrado");
        EVP_CIPHER_CTX_free(ctx);
        return STEGO_ERR_CRYPTO;
    }

    int block_size = EVP_CIPHER_block_size(cipher);
    uint8_t *ciphertext = malloc(in_len + block_size);
    if (ciphertext == NULL) {
        stego_set_error("Sin memoria para el buffer de texto cifrado");
        EVP_CIPHER_CTX_free(ctx);
        return STEGO_ERR_MEMORY;
    }

    int len1 = 0;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len1, in, (int)in_len) != 1) {
        stego_set_error("Falló el cifrado de datos (EVP_EncryptUpdate)");
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return STEGO_ERR_CRYPTO;
    }
    
    int len2 = 0;
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len1, &len2) != 1) {
        stego_set_error("Falló la finalización del cifrado (EVP_EncryptFinal_ex)");
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return STEGO_ERR_CRYPTO;
    }
    
    *out = ciphertext;
    *out_len = (size_t)(len1 + len2);
    
    EVP_CIPHER_CTX_free(ctx);
    return STEGO_OK;
}

StegoStatus crypto_decrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {

    if (password == NULL || in == NULL || out == NULL || out_len == NULL || algo < 0 || mode < 0 || algo > 3 || mode > 3) {
        stego_set_error("Parámetros inválidos al intentar descifrar");
        return STEGO_ERR_ARGS;
    }

    const EVP_CIPHER *cipher = cipher_map[algo][mode]();

    uint8_t key[EVP_MAX_KEY_LENGTH], iv_raw[EVP_MAX_IV_LENGTH];
    StegoStatus err = get_key_iv(cipher, password, key, iv_raw);
    if (err != STEGO_OK) {
        return err;
    }
    const uint8_t *iv = (EVP_CIPHER_iv_length(cipher) > 0) ? iv_raw : NULL;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        stego_set_error("No se pudo crear el contexto de descifrado");
        return STEGO_ERR_MEMORY;
    }

    if (EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv) != 1) {
        stego_set_error("Falló la inicialización del descifrado");
        EVP_CIPHER_CTX_free(ctx);
        return STEGO_ERR_CRYPTO;
    }

    int block_size = EVP_CIPHER_block_size(cipher);
    uint8_t *plaintext = malloc(in_len + block_size);
    if (plaintext == NULL) {
        stego_set_error("Sin memoria para el buffer de texto descifrado");
        EVP_CIPHER_CTX_free(ctx);
        return STEGO_ERR_MEMORY;
    }

    int len1 = 0;
    if (EVP_DecryptUpdate(ctx, plaintext, &len1, in, (int)in_len) != 1) {
        stego_set_error("Falló el descifrado de datos (EVP_DecryptUpdate)");
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return STEGO_ERR_CRYPTO;
    }

    int len2 = 0;
    if (EVP_DecryptFinal_ex(ctx, plaintext + len1, &len2) != 1) {
        stego_set_error("Falló la finalización del descifrado (posible contraseña incorrecta o datos corruptos)");
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        return STEGO_ERR_CRYPTO;
    }

    *out = plaintext;
    *out_len = (size_t)(len1 + len2);

    EVP_CIPHER_CTX_free(ctx);
    return STEGO_OK;
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
        stego_set_error("Falló la derivación de clave e IV con PBKDF2");
        return STEGO_ERR_CRYPTO; 
    }

    memcpy(key_out, key_iv_buf, key_length);
    if (iv_length > 0 && iv_out != NULL) {
        memcpy(iv_out, key_iv_buf + key_length, iv_length);
    }
    return STEGO_OK;
} 