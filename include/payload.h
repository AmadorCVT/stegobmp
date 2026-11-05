/*
 * payload.h - Format of the hidden data (size || file || extension, optionally
 * encrypted). Sits between main.c and steg.h / crypto.h.
 *
 * Without encryption the stream is:
 *     size(4, big endian) || file bytes || extension (".txt" + '\0')
 *
 * With encryption the stream is:
 *     cipher_size(4, big endian) ||
 *         encrypt(size(4, BE) || file bytes || extension)
 */
#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "bmp.h"
#include "stegobmp.h"

/* Maximum extension length INCLUDING the leading '.' and the final '\0'. */
#define PAYLOAD_EXT_MAX 32

/*
 * Extracts the extension of `path` (last '.' of the file name, e.g. ".txt")
 * into `ext`. If the name has no extension, `ext` is "" (the stream then only
 * stores the '\0'). STEGO_ERR_ARGS if it does not fit in PAYLOAD_EXT_MAX.
 */
StegoStatus payload_ext_from_path(const char *path, char ext[PAYLOAD_EXT_MAX]);

/*
 * Builds the stream to hide. `ext` is "" or ".xxx". Uses opts->password /
 * algo / mode to decide whether (and how) to encrypt. On success *stream is a
 * malloc'ed buffer owned by the caller.
 */
StegoStatus payload_build(const uint8_t *file, size_t file_size,
                          const char ext[PAYLOAD_EXT_MAX],
                          const Options *opts,
                          uint8_t **stream, size_t *stream_size);

/*
 * Reads the stream hidden in `bmp` with opts->method (via steg_extract),
 * decrypts it if opts->password is set and returns the original file and its
 * extension. On success *file is a malloc'ed buffer owned by the caller
 * (*file_size may be 0). Returns STEGO_ERR_CORRUPT when the sizes read are
 * inconsistent with the carrier capacity or the extension is missing.
 */
StegoStatus payload_extract(const Bmp *bmp, const Options *opts,
                            uint8_t **file, size_t *file_size,
                            char ext[PAYLOAD_EXT_MAX]);

#endif /* PAYLOAD_H */
