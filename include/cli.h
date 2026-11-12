/* cli.h - parseo de la linea de comandos */
#ifndef CLI_H
#define CLI_H

#include <stdio.h>

#include "stegobmp.h"

/*
 * Llena opts a partir de argv. Si hay -pass sin -a o -m usa aes128 y cbc.
 * Si falta algo o está mal devuelve STEGO_ERR_ARGS.
 */
StegoStatus cli_parse(int argc, char *argv[], Options *opts);

/* imprime como se usa */
void cli_print_usage(FILE *out);

#endif /* CLI_H */
