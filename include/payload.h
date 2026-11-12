/*
 * payload.h - formato de lo que se oculta
 *
 * sin cifrar: tamaño (4, big endian) || archivo || extension (".txt\0")
 * cifrado:    tamaño del cifrado (4, BE) || cifrar(lo de arriba)
 */
#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "bmp.h"
#include "stegobmp.h"

/* largo maximo de la extension, contando el '.' y el '\0' */
#define PAYLOAD_EXT_MAX 32

/* Saca la extensión de path (".txt"). Da error si no tiene. */
StegoStatus payload_ext_from_path(const char *path, char ext[PAYLOAD_EXT_MAX]);

/* arma el stream a ocultar, cifrado si hay password. *stream hay que liberarlo */
StegoStatus payload_build(const uint8_t *file, size_t file_size,
                          const char ext[PAYLOAD_EXT_MAX],
                          const Options *opts,
                          uint8_t **stream, size_t *stream_size);

/*
 * Interpreta lo que se recuperó del portador (descifra antes si hay password).
 * *file hay que liberarlo. Si los datos no cierran devuelve STEGO_ERR_CORRUPT.
 */
StegoStatus payload_parse(const uint8_t *stream, size_t stream_size,
                          const Options *opts,
                          uint8_t **file, size_t *file_size,
                          char ext[PAYLOAD_EXT_MAX]);

/* lee todo lo que entra en el portador y llama a payload_parse() */
StegoStatus payload_extract(const Bmp *bmp, const Options *opts,
                            uint8_t **file, size_t *file_size,
                            char ext[PAYLOAD_EXT_MAX]);

#endif /* PAYLOAD_H */
