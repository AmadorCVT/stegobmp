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
 *
 * The extension is mandatory: it starts with '.', has at least one more
 * character, and ends with '\0'. Files without extension are rejected when
 * embedding.
 */
#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "bmp.h"
#include "stegobmp.h"

/* Maximum extension length INCLUDING the leading '.' and the final '\0'. */
#define PAYLOAD_EXT_MAX 32

/*
 * Extracts the extension of `path` (last '.' of the file name, e.g. ".txt")
 * into `ext`. STEGO_ERR_ARGS if the name has no extension (also for hidden
 * files like ".bashrc") or it does not fit in PAYLOAD_EXT_MAX.
 */
StegoStatus payload_ext_from_path(const char *path, char ext[PAYLOAD_EXT_MAX]);

/*
 * Builds the stream to hide. `ext` is ".xxx". Uses opts->password / algo /
 * mode to decide whether (and how) to encrypt. On success *stream is a
 * malloc'ed buffer owned by the caller.
 */
StegoStatus payload_build(const uint8_t *file, size_t file_size,
                          const char ext[PAYLOAD_EXT_MAX],
                          const Options *opts,
                          uint8_t **stream, size_t *stream_size);

/*
 * Interprets `stream` (all the bytes recovered from the carrier, up to its
 * capacity; any bytes after the payload are ignored). Decrypts it first if
 * opts->password is set. On success *file is a malloc'ed buffer owned by the
 * caller (never NULL, *file_size may be 0) and `ext` the recovered extension.
 *
 * STEGO_ERR_CORRUPT when a size is larger than the stream or the extension is
 * missing / has no terminating '\0' (usually wrong -steg or wrong password);
 * STEGO_ERR_CRYPTO when decryption fails.
 */
StegoStatus payload_parse(const uint8_t *stream, size_t stream_size,
                          const Options *opts,
                          uint8_t **file, size_t *file_size,
                          char ext[PAYLOAD_EXT_MAX]);

/*
 * Convenience wrapper: recovers every byte the carrier can hold with
 * opts->method (steg_extract up to steg_capacity) and runs payload_parse().
 */
StegoStatus payload_extract(const Bmp *bmp, const Options *opts,
                            uint8_t **file, size_t *file_size,
                            char ext[PAYLOAD_EXT_MAX]);

#endif /* PAYLOAD_H */
