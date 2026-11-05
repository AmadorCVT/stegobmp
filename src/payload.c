/*
 * payload.c - Hidden data format. See the contract in include/payload.h.
 *
 * payload_build / payload_extract are placeholders: their owner replaces them.
 */
#include "payload.h"

#include <string.h>

StegoStatus payload_ext_from_path(const char *path, char ext[PAYLOAD_EXT_MAX]) {
    /* Look for the last '.' of the file name (after the last separator). */
    const char *name = path;
    for (const char *p = path; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            name = p + 1;
        }
    }
    const char *dot = strrchr(name, '.');
    if (dot == NULL || dot == name) { /* no extension, or hidden file ".x" */
        ext[0] = '\0';
        return STEGO_OK;
    }

    size_t len = strlen(dot);
    if (len + 1 > PAYLOAD_EXT_MAX) {
        stego_set_error("la extensión '%s' es demasiado larga (máximo %d "
                        "caracteres)", dot, PAYLOAD_EXT_MAX - 1);
        return STEGO_ERR_ARGS;
    }
    memcpy(ext, dot, len + 1);
    return STEGO_OK;
}

StegoStatus payload_build(const uint8_t *file, size_t file_size,
                          const char ext[PAYLOAD_EXT_MAX],
                          const Options *opts,
                          uint8_t **stream, size_t *stream_size) {
    (void)file; (void)file_size; (void)ext; (void)opts;
    (void)stream; (void)stream_size;
    return stego_not_implemented("armado del payload");
}

StegoStatus payload_extract(const Bmp *bmp, const Options *opts,
                            uint8_t **file, size_t *file_size,
                            char ext[PAYLOAD_EXT_MAX]) {
    (void)bmp; (void)opts; (void)file; (void)file_size; (void)ext;
    return stego_not_implemented("lectura del payload");
}
