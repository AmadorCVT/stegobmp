/* crypto.c - cifrado y descifrado con OpenSSL */

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
 * saca la key y el iv de la password con PBKDF2-HMAC-SHA256.
 * el iv solo se copia si el modo lo usa (en ECB no hay)
 */
static StegoStatus get_key_iv(const EVP_CIPHER *cipher, const char *password, uint8_t *key_out, uint8_t *iv_out);

/* cifra (enc = 1) o descifra (enc = 0). *out hay que liberarlo */
static StegoStatus crypto_run(int enc, CipherAlgo algo, CipherMode mode,
                              const char *password,
                              const uint8_t *in, size_t in_len,
                              uint8_t **out, size_t *out_len);


StegoStatus crypto_encrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {
    return crypto_run(1, algo, mode, password, in, in_len, out, out_len);
}

StegoStatus crypto_decrypt(CipherAlgo algo, CipherMode mode,
                           const char *password,
                           const uint8_t *in, size_t in_len,
                           uint8_t **out, size_t *out_len) {
    return crypto_run(0, algo, mode, password, in, in_len, out, out_len);
}


static StegoStatus crypto_run(int enc, CipherAlgo algo, CipherMode mode,
                              const char *password,
                              const uint8_t *in, size_t in_len,
                              uint8_t **out, size_t *out_len) {
    const char *verb = enc ? "cifrar" : "descifrar";
    StegoStatus st = STEGO_OK;
    EVP_CIPHER_CTX *ctx = NULL;
    uint8_t *buf = NULL;

    if (password == NULL || in == NULL || out == NULL || out_len == NULL || algo < 0 || mode < 0 || algo > 3 || mode > 3) {
        stego_set_error("Parámetros inválidos al intentar %s", verb);
        return STEGO_ERR_ARGS;
    }

    const EVP_CIPHER *cipher = cipher_map[algo][mode]();

    uint8_t key[EVP_MAX_KEY_LENGTH], iv_raw[EVP_MAX_IV_LENGTH];
    st = get_key_iv(cipher, password, key, iv_raw);
    if (st != STEGO_OK) {
        return st;
    }
    const uint8_t *iv = (EVP_CIPHER_iv_length(cipher) > 0) ? iv_raw : NULL;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        stego_set_error("No se pudo crear el contexto para %s", verb);
        st = STEGO_ERR_MEMORY;
        goto cleanup;
    }

    // EVP_CipherInit_ex hace lo mismo que EVP_EncryptInit_ex / EVP_DecryptInit_ex segun enc
    if (EVP_CipherInit_ex(ctx, cipher, NULL, key, iv, enc) != 1) {
        stego_set_error("Falló la inicialización al %s", verb);
        st = STEGO_ERR_CRYPTO;
        goto cleanup;
    }

    // con padding la salida puede tener hasta un bloque mas que la entrada
    buf = malloc(in_len + EVP_CIPHER_block_size(cipher));
    if (buf == NULL) {
        stego_set_error("Sin memoria para el buffer al %s", verb);
        st = STEGO_ERR_MEMORY;
        goto cleanup;
    }

    int len1 = 0;
    if (EVP_CipherUpdate(ctx, buf, &len1, in, (int)in_len) != 1) {
        stego_set_error("Falló al %s los datos (EVP_CipherUpdate)", verb);
        st = STEGO_ERR_CRYPTO;
        goto cleanup;
    }

    int len2 = 0;
    if (EVP_CipherFinal_ex(ctx, buf + len1, &len2) != 1) {
        if (enc) {
            stego_set_error("Falló la finalización del cifrado (EVP_CipherFinal_ex)");
        } else {
            stego_set_error("Falló la finalización del descifrado (posible contraseña incorrecta o datos corruptos)");
        }
        st = STEGO_ERR_CRYPTO;
        goto cleanup;
    }

    *out = buf;
    *out_len = (size_t)(len1 + len2);
    buf = NULL;     // ahora es de quien llamo, no se libera aca

cleanup:
    free(buf);
    EVP_CIPHER_CTX_free(ctx);   // acepta NULL
    return st;
}


static StegoStatus get_key_iv(const EVP_CIPHER *cipher, const char *password, uint8_t *key_out, uint8_t *iv_out){

    uint8_t salt[8] = PBKDF2_SALT;
    int key_length = EVP_CIPHER_key_length(cipher);
    int iv_length = EVP_CIPHER_iv_length(cipher);
    int total_length = key_length + iv_length;
    uint8_t key_iv_buf[EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH];

    // int PKCS5_PBKDF2_HMAC(const char *pass, int passlen, const unsigned char *salt, int saltlen, int iter, const EVP_MD *digest, int keylen, unsigned char *out);
    // genera key+iv con la password, salt = 0x00 (8), 10000 iteraciones, sha256
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
