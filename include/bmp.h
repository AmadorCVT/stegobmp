/* bmp.h - leer y escribir BMP (V3, 24 bits, sin compresion) */
#ifndef BMP_H
#define BMP_H

#include "stegobmp.h"

#define BMP_HEADER_SIZE 54

/*
 * header: todo lo que esta antes de bfOffBits, se guarda tal cual.
 * data: el cuerpo (con el padding de cada fila), aca es donde se oculta.
 */
typedef struct {
    uint8_t *header;
    size_t   header_size;
    uint8_t *data;
    size_t   data_size;
    uint32_t width;           /* en pixeles */
    uint32_t height;          /* valor absoluto de biHeight */
} Bmp;

/* Carga y valida el BMP. Si anda hay que llamar a bmp_free() después. */
StegoStatus bmp_load(const char *path, Bmp *bmp);

/* escribe header + data */
StegoStatus bmp_save(const char *path, const Bmp *bmp);

/* capacidad en bytes segun el metodo (ver steg.h) */
size_t bmp_capacity_bytes(const Bmp *bmp, StegMethod method);

/* libera todo, se puede llamar con un Bmp en cero */
void bmp_free(Bmp *bmp);

#endif /* BMP_H */
