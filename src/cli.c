#include "cli.h"

#include <string.h>

/* opciones que conoce el programa, sirve para darse cuenta si falta un valor */
static const char *const KNOWN_OPTIONS[] = {
    "-embed", "-extract", "-in", "-p", "-out", "-steg", "-a", "-m", "-pass"
};

static bool is_known_option(const char *s) {
    for (size_t i = 0; i < sizeof KNOWN_OPTIONS / sizeof KNOWN_OPTIONS[0]; i++) {
        if (strcmp(s, KNOWN_OPTIONS[i]) == 0) {
            return true;
        }
    }
    return false;
}

/* guion largo en UTF-8 (E2 80 93 o E2 80 94), pasa si se copia del pdf */
static bool starts_with_long_dash(const char *s) {
    return (unsigned char)s[0] == 0xE2 && (unsigned char)s[1] == 0x80 &&
           ((unsigned char)s[2] == 0x93 || (unsigned char)s[2] == 0x94);
}

/* Busca value en names. Devuelve el índice, o -1 si no está. */
static int lookup(const char *const names[], size_t count, const char *value) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(value, names[i]) == 0) {
            return (int)i;
        }
    }
    return -1;
}

/* devuelve el valor que sigue a argv[*i] y avanza i, o NULL si no hay */
static const char *take_value(int argc, char *argv[], int *i) {
    const char *opt = argv[*i];
    if (*i + 1 >= argc) {
        stego_set_error("falta el valor del parámetro %s", opt);
        return NULL;
    }
    const char *value = argv[*i + 1];
    if (is_known_option(value)) {
        stego_set_error("falta el valor del parámetro %s (se encontró '%s')",
                        opt, value);
        return NULL;
    }
    if (value[0] == '\0') {
        stego_set_error("el valor del parámetro %s no puede estar vacío", opt);
        return NULL;
    }
    (*i)++;
    return value;
}

#define NAMES(arr) arr, sizeof arr / sizeof arr[0]

StegoStatus cli_parse(int argc, char *argv[], Options *opts) {
    static const char *const methods[] = {"LSB1", "LSB4", "LSBI"};
    static const char *const algos[]   = {"aes128", "aes192", "aes256", "3des"};
    static const char *const modes[]   = {"ecb", "cfb", "ofb", "cbc"};

    memset(opts, 0, sizeof *opts);
    opts->algo = CIPHER_AES128;
    opts->mode = MODE_CBC;

    bool seen_embed = false, seen_extract = false;
    bool seen_in = false, seen_p = false, seen_out = false, seen_steg = false;
    bool seen_a = false, seen_m = false, seen_pass = false;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        const char *value;

#define REJECT_DUPLICATE(seen)                                              \
    do {                                                                    \
        if (seen) {                                                         \
            stego_set_error("el parámetro %s está repetido", arg);          \
            return STEGO_ERR_ARGS;                                          \
        }                                                                   \
        seen = true;                                                        \
    } while (0)

#define TAKE_VALUE()                                                        \
    do {                                                                    \
        value = take_value(argc, argv, &i);                                 \
        if (value == NULL) {                                                \
            return STEGO_ERR_ARGS;                                          \
        }                                                                   \
    } while (0)

        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            opts->show_help = true;
            return STEGO_OK;
        } else if (strcmp(arg, "-embed") == 0) {
            REJECT_DUPLICATE(seen_embed);
            opts->op = OP_EMBED;
        } else if (strcmp(arg, "-extract") == 0) {
            REJECT_DUPLICATE(seen_extract);
            opts->op = OP_EXTRACT;
        } else if (strcmp(arg, "-in") == 0) {
            REJECT_DUPLICATE(seen_in);
            TAKE_VALUE();
            opts->in_path = value;
        } else if (strcmp(arg, "-p") == 0) {
            REJECT_DUPLICATE(seen_p);
            TAKE_VALUE();
            opts->carrier_path = value;
        } else if (strcmp(arg, "-out") == 0) {
            REJECT_DUPLICATE(seen_out);
            TAKE_VALUE();
            opts->out_path = value;
        } else if (strcmp(arg, "-steg") == 0) {
            REJECT_DUPLICATE(seen_steg);
            TAKE_VALUE();
            int idx = lookup(NAMES(methods), value);
            if (idx < 0) {
                stego_set_error("valor inválido '%s' para -steg (use LSB1, "
                                "LSB4 o LSBI)", value);
                return STEGO_ERR_ARGS;
            }
            opts->method = (StegMethod)idx;
        } else if (strcmp(arg, "-a") == 0) {
            REJECT_DUPLICATE(seen_a);
            TAKE_VALUE();
            int idx = lookup(NAMES(algos), value);
            if (idx < 0) {
                stego_set_error("valor inválido '%s' para -a (use aes128, "
                                "aes192, aes256 o 3des)", value);
                return STEGO_ERR_ARGS;
            }
            opts->algo = (CipherAlgo)idx;
        } else if (strcmp(arg, "-m") == 0) {
            REJECT_DUPLICATE(seen_m);
            TAKE_VALUE();
            int idx = lookup(NAMES(modes), value);
            if (idx < 0) {
                stego_set_error("valor inválido '%s' para -m (use ecb, cfb, "
                                "ofb o cbc)", value);
                return STEGO_ERR_ARGS;
            }
            opts->mode = (CipherMode)idx;
        } else if (strcmp(arg, "-pass") == 0) {
            REJECT_DUPLICATE(seen_pass);
            TAKE_VALUE();
            opts->password = value;
        } else if (starts_with_long_dash(arg)) {
            stego_set_error("parámetro '%s' inválido: se usó un guion largo; "
                            "use el guion común '-'", arg);
            return STEGO_ERR_ARGS;
        } else if (arg[0] == '-') {
            stego_set_error("parámetro desconocido '%s'", arg);
            return STEGO_ERR_ARGS;
        } else {
            stego_set_error("argumento inesperado '%s'", arg);
            return STEGO_ERR_ARGS;
        }
    }

#undef REJECT_DUPLICATE
#undef TAKE_VALUE

    if (seen_embed && seen_extract) {
        stego_set_error("-embed y -extract no pueden usarse juntos");
        return STEGO_ERR_ARGS;
    }
    if (!seen_embed && !seen_extract) {
        stego_set_error("falta indicar la operación: -embed o -extract");
        return STEGO_ERR_ARGS;
    }

    if (opts->op == OP_EMBED && !seen_in) {
        stego_set_error("falta el parámetro -in");
        return STEGO_ERR_ARGS;
    }
    if (opts->op == OP_EXTRACT && seen_in) {
        stego_set_error("el parámetro -in no corresponde a -extract");
        return STEGO_ERR_ARGS;
    }
    if (!seen_p) {
        stego_set_error("falta el parámetro -p");
        return STEGO_ERR_ARGS;
    }
    if (!seen_out) {
        stego_set_error("falta el parámetro -out");
        return STEGO_ERR_ARGS;
    }
    if (!seen_steg) {
        stego_set_error("falta el parámetro -steg");
        return STEGO_ERR_ARGS;
    }

    if (!seen_pass && (seen_a || seen_m)) {
        stego_set_error("%s%s%s requiere%s -pass: no se puede cifrar sin "
                        "password",
                        seen_a ? "-a" : "", seen_a && seen_m ? " y " : "",
                        seen_m ? "-m" : "", seen_a && seen_m ? "n" : "");
        return STEGO_ERR_ARGS;
    }

    return STEGO_OK;
}

void cli_print_usage(FILE *out) {
    fprintf(out,
        "Uso:\n"
        "  stegobmp -embed -in file -p bitmapfile -out bitmapfile\n"
        "           -steg <LSB1 | LSB4 | LSBI>\n"
        "           [-a <aes128 | aes192 | aes256 | 3des>]\n"
        "           [-m <ecb | cfb | ofb | cbc>] [-pass password]\n"
        "\n"
        "  stegobmp -extract -p bitmapfile -out file\n"
        "           -steg <LSB1 | LSB4 | LSBI>\n"
        "           [-a <aes128 | aes192 | aes256 | 3des>]\n"
        "           [-m <ecb | cfb | ofb | cbc>] [-pass password]\n"
        "\n"
        "Notas:\n"
        "  - Sin -pass no se cifra. -a y -m requieren -pass.\n"
        "  - Con -pass y sin -a se usa aes128; sin -m se usa cbc.\n"
        "  - Los valores distinguen mayúsculas y minúsculas.\n"
        "  - En -extract, -out es el nombre sin extensión: se le agrega la\n"
        "    extensión del archivo oculto.\n");
}
