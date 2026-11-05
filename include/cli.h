/*
 * cli.h - Command line parsing.
 */
#ifndef CLI_H
#define CLI_H

#include <stdio.h>

#include "stegobmp.h"

/*
 * Parses argv into `opts` (options in any order, case sensitive, only the
 * plain '-' prefix). Applies defaults: no -pass => no encryption; -pass
 * without -a => aes128; -pass without -m => cbc.
 *
 * Returns STEGO_ERR_ARGS with a message in stego_error_message() when a
 * parameter is missing, repeated, unexpected or has an invalid value, or when
 * -a / -m are used without -pass. `--help` / `-h` sets opts->show_help.
 */
StegoStatus cli_parse(int argc, char *argv[], Options *opts);

/* Prints the correct syntax of the program. */
void cli_print_usage(FILE *out);

#endif /* CLI_H */
