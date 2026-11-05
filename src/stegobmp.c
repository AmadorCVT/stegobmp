#include "stegobmp.h"

#include <stdarg.h>
#include <stdio.h>

/* ------------------------------------------------------------------------ */
/* Error message                                                            */
/* ------------------------------------------------------------------------ */

static char last_error[512] = "";

void stego_set_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(last_error, sizeof last_error, fmt, ap);
    va_end(ap);
}

const char *stego_error_message(void) {
    return last_error;
}

StegoStatus stego_not_implemented(const char *what) {
    stego_set_error("%s: no implementado todavía", what);
    return STEGO_ERR_NOT_IMPLEMENTED;
}

/* ------------------------------------------------------------------------ */
/* Names (same order as the enums in stegobmp.h)                            */
/* ------------------------------------------------------------------------ */

static const char *const METHOD_NAMES[] = {"LSB1", "LSB4", "LSBI"};
static const char *const ALGO_NAMES[]   = {"aes128", "aes192", "aes256", "3des"};
static const char *const MODE_NAMES[]   = {"ecb", "cfb", "ofb", "cbc"};

const char *stego_method_name(StegMethod method) {
    return METHOD_NAMES[method];
}

const char *cipher_algo_name(CipherAlgo algo) {
    return ALGO_NAMES[algo];
}

const char *cipher_mode_name(CipherMode mode) {
    return MODE_NAMES[mode];
}
