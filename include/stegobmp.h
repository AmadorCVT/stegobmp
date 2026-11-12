/* stegobmp.h - tipos que usan todos los modulos */
#ifndef STEGOBMP_H
#define STEGOBMP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Si algo falla se llama a stego_set_error() antes de devolver el error,
 * main solo imprime ese mensaje. */
typedef enum {
    STEGO_OK = 0,
    STEGO_ERR_ARGS,            /* parametros invalidos */
    STEGO_ERR_IO,              /* no se pudo abrir/leer/escribir */
    STEGO_ERR_FORMAT,          /* bmp no soportado o roto */
    STEGO_ERR_CAPACITY,        /* no entra en el portador */
    STEGO_ERR_CRYPTO,          /* fallo el cifrado (ej: password mal) */
    STEGO_ERR_CORRUPT,         /* lo oculto no tiene sentido */
    STEGO_ERR_MEMORY           /* sin memoria */
} StegoStatus;

/* el orden tiene que coincidir con las tablas de nombres de stegobmp.c y cli.c */
typedef enum {
    STEG_LSB1 = 0,
    STEG_LSB4,
    STEG_LSBI
} StegMethod;

typedef enum {
    CIPHER_AES128 = 0,
    CIPHER_AES192,
    CIPHER_AES256,
    CIPHER_3DES            /* des-ede3, 3 claves */
} CipherAlgo;

typedef enum {
    MODE_ECB = 0,
    MODE_CFB,              /* cfb8 */
    MODE_OFB,              /* ofb de 128 bits */
    MODE_CBC
} CipherMode;

typedef enum {
    OP_NONE = 0,
    OP_EMBED,
    OP_EXTRACT
} Operation;

/*
 * Lo que devuelve cli_parse(). Los strings apuntan a argv.
 * En extract, out_path va sin extensión. Si password es NULL no se cifra.
 */
typedef struct {
    bool        show_help;
    Operation   op;
    const char *in_path;
    const char *carrier_path;
    const char *out_path;
    StegMethod  method;
    const char *password;
    CipherAlgo  algo;
    CipherMode  mode;
} Options;

/* true si hay que cifrar/descifrar */
static inline bool options_use_crypto(const Options *opts) {
    return opts->password != NULL;
}

#if defined(__GNUC__)
#define STEGO_PRINTF_LIKE(fmt, args) __attribute__((format(printf, fmt, args)))
#else
#define STEGO_PRINTF_LIKE(fmt, args)
#endif

/* guarda el mensaje del ultimo error (como printf) */
void stego_set_error(const char *fmt, ...) STEGO_PRINTF_LIKE(1, 2);

/* Último mensaje guardado, "" si no hubo error. */
const char *stego_error_message(void);

/* nombres tal cual se escriben en la linea de comandos */
const char *stego_method_name(StegMethod method);
const char *cipher_algo_name(CipherAlgo algo);
const char *cipher_mode_name(CipherMode mode);

#endif /* STEGOBMP_H */
