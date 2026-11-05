/*
 * stegobmp.h - Common types shared by every module.
 *
 * CONTRACT: changing anything in this header (or in bmp.h, payload.h, steg.h,
 * crypto.h) requires agreement of the whole group.
 */
#ifndef STEGOBMP_H
#define STEGOBMP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ------------------------------------------------------------------------ */
/* Status codes                                                             */
/* ------------------------------------------------------------------------ */

/*
 * Every function that can fail returns a StegoStatus. On failure it must also
 * call stego_set_error() with a human readable (Spanish) message; main() just
 * prints stego_error_message(), so the message must be self-explanatory.
 */
typedef enum {
    STEGO_OK = 0,
    STEGO_ERR_ARGS,            /* invalid command line / invalid argument */
    STEGO_ERR_IO,              /* cannot open / read / write a file */
    STEGO_ERR_FORMAT,          /* unsupported or malformed BMP */
    STEGO_ERR_CAPACITY,        /* the carrier is too small */
    STEGO_ERR_CRYPTO,          /* cipher failure (e.g. wrong password) */
    STEGO_ERR_CORRUPT,         /* hidden data is inconsistent */
    STEGO_ERR_MEMORY,          /* out of memory */
    STEGO_ERR_NOT_IMPLEMENTED  /* placeholder body, not written yet */
} StegoStatus;

/* ------------------------------------------------------------------------ */
/* Enumerations (the order matches the name tables in src/stegobmp.c)       */
/* ------------------------------------------------------------------------ */

typedef enum {
    STEG_LSB1 = 0,
    STEG_LSB4,
    STEG_LSBI
} StegMethod;

typedef enum {
    CIPHER_AES128 = 0,
    CIPHER_AES192,
    CIPHER_AES256,
    CIPHER_3DES            /* des-ede3 (three different keys) */
} CipherAlgo;

typedef enum {
    MODE_ECB = 0,
    MODE_CFB,              /* 8 bit feedback (cfb8) */
    MODE_OFB,              /* 128 bit feedback (plain ofb) */
    MODE_CBC
} CipherMode;

typedef enum {
    OP_NONE = 0,
    OP_EMBED,
    OP_EXTRACT
} Operation;

/* ------------------------------------------------------------------------ */
/* Command line options                                                     */
/* ------------------------------------------------------------------------ */

/*
 * Result of cli_parse(). Strings point into argv (not owned).
 *
 *   in_path       -in    (OP_EMBED only, NULL for OP_EXTRACT)
 *   carrier_path  -p
 *   out_path      -out   (embed: output BMP; extract: output file WITHOUT
 *                         extension, the recovered extension is appended)
 *   method        -steg
 *   password      -pass  (NULL means "do not encrypt")
 *   algo / mode   -a / -m. Defaults already applied by the parser:
 *                 AES-128 and CBC. Only meaningful when password != NULL.
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

/* true when the payload must be encrypted / decrypted */
static inline bool options_use_crypto(const Options *opts) {
    return opts->password != NULL;
}

/* ------------------------------------------------------------------------ */
/* Error reporting and name helpers (implemented in src/stegobmp.c)         */
/* ------------------------------------------------------------------------ */

#if defined(__GNUC__)
#define STEGO_PRINTF_LIKE(fmt, args) __attribute__((format(printf, fmt, args)))
#else
#define STEGO_PRINTF_LIKE(fmt, args)
#endif

/* Stores a printf-style message describing the last error. */
void stego_set_error(const char *fmt, ...) STEGO_PRINTF_LIKE(1, 2);

/* Last message stored with stego_set_error() ("" if none). */
const char *stego_error_message(void);

/*
 * Convenience for placeholder bodies: sets the message "<what>: no
 * implementado todavía" and returns STEGO_ERR_NOT_IMPLEMENTED.
 */
StegoStatus stego_not_implemented(const char *what);

/* Names exactly as typed on the command line (LSB1, aes128, cbc, ...). */
const char *stego_method_name(StegMethod method);
const char *cipher_algo_name(CipherAlgo algo);
const char *cipher_mode_name(CipherMode mode);

#endif /* STEGOBMP_H */
