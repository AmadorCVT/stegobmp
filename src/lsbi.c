/* lsbi.c - LSB Improved (Majeed y Sulaiman) */
#include "steg.h"

#define GROUP_MASK 0x06
#define LAST_BIT_MASK 0x01
#define GROUPS 4

/* grupo del byte: sus bits 1 y 2 (no cambian al tocar el LSB) */
static inline uint8_t pattern(uint8_t byte) {
    return (uint8_t)((byte & GROUP_MASK) >> 1);
}

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

        uint8_t bit = msg_bit(data, i);
        uint8_t byte = bmp->data[i + GROUPS];

        if (bit == (byte & LAST_BIT_MASK)) {
            count_unmodified[pattern(byte)]++;
        } else {
            count_modified[pattern(byte)]++;
        }
    }

    // vemos entonces que grupos hay que reemplazar
    for (int i = 0; i < GROUPS; i++) {
        inverted[i] = count_modified[i] > count_unmodified[i];
        bmp->data[i] = set_lsb(bmp->data[i], inverted[i]);
    }

    // ahora llenamos 
    for(size_t i = 0; i < len * 8; i++) {
        uint8_t byte = bmp->data[i + GROUPS];
        uint8_t bit = msg_bit(data, i) ^ inverted[pattern(byte)];
        bmp->data[i + GROUPS] = set_lsb(byte, bit);
    }



    return STEGO_OK;
}



    


StegoStatus lsbi_extract(const Bmp *bmp, size_t len, uint8_t *out) {



    
}
