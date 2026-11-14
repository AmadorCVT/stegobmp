/* lsbi.c - LSB Improved (Majeed y Sulaiman) */
#include "steg.h"

#define GROUP_MASK 0x06
#define LAST_BIT_MASK 0x01
#define GROUPS 4

/*
La idea es hacer un lsb, teniendo la cover image y la stego image
agrupar los bytes segun su 2do y 3er bit menos significativos (4 grupos: '00', '01', '10', '11')
fijarse por cada byte si el lsb cambio, si en un determinado grupo cambio mas de lo que se mantuvo -> lo invertimos en la cover image

hay que agregar al principio cuales van a estar invertidos y cuales no
*/

StegoStatus lsbi_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    size_t count_modified[GROUPS] = {0};
    size_t count_unmodified[GROUPS] = {0};
    uint8_t inverted[GROUPS];

    // recorremos y vamos contando cuales tenemos que modificar y cuales no
    for(size_t i = 0; i < len * 8; i++){

        uint8_t bit = (data[i / 8] >> (7 - i % 8)) & 1;

        uint8_t group = (bmp->data[i + GROUPS] & GROUP_MASK) >> 1;

        if (bit == (bmp->data[i + GROUPS] & LAST_BIT_MASK)) {
            count_unmodified[group]++;
        } else {
            count_modified[group]++;
        }
    }

    // vemos entonces que grupos hay que reemplazar
    for (int i = 0; i < GROUPS; i++) {
        inverted[i] = count_modified[i] > count_unmodified[i];
        uint8_t bit = inverted[i];
        bmp->data[i] = (bmp->data[i] & 0xFE) | bit;
    }

    // ahora llenamos 
    for(size_t i = 0; i < len * 8; i++) {
        uint8_t bit = (data[i / 8] >> (7 - i % 8)) & 0x01;
        uint8_t group = (bmp->data[i + GROUPS] & GROUP_MASK) >> 1;
        bit ^= inverted[group];
        bmp->data[i + GROUPS] = (bmp->data[i + GROUPS] & 0xFE) | bit;
    }



    return STEGO_OK;
}



    


StegoStatus lsbi_extract(const Bmp *bmp, size_t len, uint8_t *out) {
}
