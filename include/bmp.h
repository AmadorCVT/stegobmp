/*
 * bmp.h - BMP (version V3, 24 bpp, uncompressed) loading and saving.
 */
#ifndef BMP_H
#define BMP_H

#include "stegobmp.h"

#define BMP_HEADER_SIZE 54

/*
 * A loaded BMP, split in the two regions that matter for steganography.
 *
 *   header  bytes [0, bfOffBits) of the file, kept verbatim so bmp_save() can
 *           write them back unchanged. header_size == bfOffBits (>= 54).
 *   data    bytes [bfOffBits, end of file). This is the carrier: pixel bytes
 *           in file order (B, G, R, B, G, R, ...) INCLUDING the padding of
 *           every row. Steganography modules modify it in place.
 */
typedef struct {
    uint8_t *header;
    size_t   header_size;
    uint8_t *data;
    size_t   data_size;
    uint32_t width;           /* pixels */
    uint32_t height;          /* pixels (absolute value of biHeight) */
} Bmp;

/*
 * Loads and validates `path`. Rejects (STEGO_ERR_FORMAT, with a message):
 *   - missing "BM" signature or file shorter than the 54 byte header,
 *   - biBitCount != 24,
 *   - biCompression != 0,
 *   - bfOffBits outside [54, file size], or a body smaller than the pixel
 *     rows declared by the header.
 * STEGO_ERR_IO if the file cannot be opened/read, STEGO_ERR_MEMORY on OOM.
 * On success the caller must call bmp_free(). On failure *bmp is zeroed.
 */
StegoStatus bmp_load(const char *path, Bmp *bmp);

/*
 * Writes bmp->header followed by bmp->data to `path`. With an unmodified Bmp
 * the output is byte-identical to the file that was loaded.
 */
StegoStatus bmp_save(const char *path, const Bmp *bmp);

/* Releases the buffers and zeroes the struct. Safe on a zeroed Bmp. */
void bmp_free(Bmp *bmp);

#endif /* BMP_H */
