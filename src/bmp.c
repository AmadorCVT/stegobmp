#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t read_u16(const uint8_t *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t read_u32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Tamaño del archivo, o -1 si falla. Deja el puntero al principio. */
static long file_length(FILE *f) {
    if (fseek(f, 0, SEEK_END) != 0) {
        return -1;
    }
    long len = ftell(f);
    if (len < 0 || fseek(f, 0, SEEK_SET) != 0) {
        return -1;
    }
    return len;
}

size_t bmp_capacity_bytes(const Bmp *bmp, StegMethod method) {
    switch (method) {
    case STEG_LSB1:
        return bmp->data_size / 8;
    case STEG_LSB4:
        return bmp->data_size / 2;
    case STEG_LSBI:
        return bmp->data_size < 4 ? 0 : (bmp->data_size - 4) / 8;
    }
    return 0;
}

void bmp_free(Bmp *bmp) {
    free(bmp->header);
    free(bmp->data);
    memset(bmp, 0, sizeof *bmp);
}

StegoStatus bmp_load(const char *path, Bmp *bmp) {
    memset(bmp, 0, sizeof *bmp);

    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        stego_set_error("no se pudo abrir el BMP '%s' para lectura", path);
        return STEGO_ERR_IO;
    }

    StegoStatus st = STEGO_ERR_IO;
    uint8_t fixed[BMP_HEADER_SIZE];
    long file_size = file_length(f);
    if (file_size < 0) {
        stego_set_error("no se pudo determinar el tamaño de '%s'", path);
        goto fail;
    }

    st = STEGO_ERR_FORMAT;
    if (file_size < BMP_HEADER_SIZE) {
        stego_set_error("'%s' no es un BMP válido: mide %ld bytes y el "
                        "encabezado requiere %d", path, file_size,
                        BMP_HEADER_SIZE);
        goto fail;
    }
    if (fread(fixed, 1, BMP_HEADER_SIZE, f) != BMP_HEADER_SIZE) {
        st = STEGO_ERR_IO;
        stego_set_error("error leyendo el encabezado de '%s'", path);
        goto fail;
    }
    if (fixed[0] != 'B' || fixed[1] != 'M') {
        stego_set_error("'%s' no es un BMP: falta la firma 'BM'", path);
        goto fail;
    }

    uint32_t off_bits    = read_u32(fixed + 10);
    uint32_t info_size   = read_u32(fixed + 14);
    int32_t  width       = (int32_t)read_u32(fixed + 18);
    int32_t  height      = (int32_t)read_u32(fixed + 22);
    uint16_t bpp         = read_u16(fixed + 28);
    uint32_t compression = read_u32(fixed + 30);

    if (info_size < 40) {
        stego_set_error("'%s': encabezado BMP no soportado (se requiere "
                        "versión V3, biSize >= 40, y tiene %u)", path,
                        info_size);
        goto fail;
    }
    if (bpp != 24) {
        stego_set_error("'%s': se requieren 24 bits por píxel y el BMP tiene "
                        "%u", path, (unsigned)bpp);
        goto fail;
    }
    if (compression != 0) {
        stego_set_error("'%s': el BMP está comprimido (biCompression = %u); "
                        "solo se aceptan BMP sin compresión", path,
                        compression);
        goto fail;
    }
    if (width <= 0 || height == 0 || height == INT32_MIN) {
        stego_set_error("'%s': dimensiones inválidas (%d x %d)", path,
                        (int)width, (int)height);
        goto fail;
    }
    if (off_bits < BMP_HEADER_SIZE || (long)off_bits > file_size) {
        stego_set_error("'%s': bfOffBits inválido (%u) para un archivo de %ld "
                        "bytes", path, off_bits, file_size);
        goto fail;
    }

    uint32_t abs_height = (uint32_t)(height < 0 ? -height : height);
    uint64_t row_size   = ((uint64_t)width * 3 + 3) & ~(uint64_t)3;
    uint64_t body_min   = row_size * abs_height;
    size_t   data_size  = (size_t)file_size - off_bits;
    if (data_size < body_min) {
        stego_set_error("'%s': el BMP está truncado (el cuerpo mide %zu bytes "
                        "y el encabezado declara %llu)", path, data_size,
                        (unsigned long long)body_min);
        goto fail;
    }

    st = STEGO_ERR_MEMORY;
    bmp->header = malloc(off_bits);
    bmp->data   = malloc(data_size > 0 ? data_size : 1);
    if (bmp->header == NULL || bmp->data == NULL) {
        stego_set_error("sin memoria para cargar '%s'", path);
        goto fail;
    }

    /* el header es todo lo que esta antes de bfOffBits: los 54 bytes que ya
     * leimos mas lo que haya entre eso y el cuerpo */
    memcpy(bmp->header, fixed, BMP_HEADER_SIZE);
    st = STEGO_ERR_IO;
    if (fread(bmp->header + BMP_HEADER_SIZE, 1, off_bits - BMP_HEADER_SIZE,
              f) != off_bits - BMP_HEADER_SIZE ||
        fread(bmp->data, 1, data_size, f) != data_size) {
        stego_set_error("error leyendo '%s'", path);
        goto fail;
    }

    bmp->header_size = off_bits;
    bmp->data_size   = data_size;
    bmp->width       = (uint32_t)width;
    bmp->height      = abs_height;
    fclose(f);
    return STEGO_OK;

fail:
    fclose(f);
    bmp_free(bmp);
    return st;
}

StegoStatus bmp_save(const char *path, const Bmp *bmp) {
    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        stego_set_error("no se pudo abrir '%s' para escritura", path);
        return STEGO_ERR_IO;
    }

    bool ok = fwrite(bmp->header, 1, bmp->header_size, f) == bmp->header_size &&
              fwrite(bmp->data, 1, bmp->data_size, f) == bmp->data_size;
    if (fclose(f) != 0) {
        ok = false;
    }
    if (!ok) {
        stego_set_error("error escribiendo '%s'", path);
        return STEGO_ERR_IO;
    }
    return STEGO_OK;
}
