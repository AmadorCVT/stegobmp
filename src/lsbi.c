/* lsbi.c - LSB Improved (Majeed y Sulaiman) */
#include "steg.h"

#define PATTERN_MASK 0x06
#define PATTERNS 4

/* grupo del byte: sus bits 1 y 2 (no cambian al tocar el LSB) */
static inline uint8_t pattern(uint8_t byte) {
    return (uint8_t)((byte & PATTERN_MASK) >> 1);
}

/*
 * El mensaje va solo en azul y verde (el paper saltea el rojo).
 * En el cuerpo del bmp el orden es B, G, R, asi que los rojos son los
 * indices j con j % 3 == 2. Devuelve el primer indice usable desde j.
 */
static inline size_t skip_if_red(size_t j) {
    return (j % 3 == 2) ? j + 1 : j;
}

/*
La idea es hacer un lsb, teniendo la cover image y la stego image
agrupar los bytes segun su 2do y 3er bit menos significativos (4 grupos (patterns): '00', '01', '10', '11')
fijarse por cada byte si el lsb cambio, si en un determinado grupo cambio mas de lo que se mantuvo -> lo invertimos en la cover image

hay que agregar al principio cuales van a estar invertidos y cuales no
*/

StegoStatus lsbi_embed(Bmp *bmp, const uint8_t *data, size_t len) {
    size_t count_modified[PATTERNS] = {0};
    size_t count_unmodified[PATTERNS] = {0};
    uint8_t inverted[PATTERNS];

    // recorremos y vamos contando cuales tenemos que modificar y cuales no
    // j recorre la imagen (salteando rojos), i recorre los bits del mensaje
    
    for(size_t i = 0, j = PATTERNS; i < len * 8; i++, j++){
        j = skip_if_red(j);
        uint8_t bit = msg_bit(data, i);
        uint8_t byte = bmp->data[j];

        if (bit == get_lsb(byte)) {
            count_unmodified[pattern(byte)]++;
        } else {
            count_modified[pattern(byte)]++;
        }

        
    }

    // vemos entonces que grupos hay que reemplazar
    for (int i = 0; i < PATTERNS; i++) {
        inverted[i] = count_modified[i] > count_unmodified[i];
        bmp->data[i] = set_lsb(bmp->data[i], inverted[i]);
    }

    // ahora llenamos 
    for(size_t i = 0, j = PATTERNS; i < len * 8; i++, j++) {
        j = skip_if_red(j);
        uint8_t byte = bmp->data[j];
        uint8_t bit = msg_bit(data, i) ^ inverted[pattern(byte)];
        bmp->data[j] = set_lsb(byte, bit);
    }

    return STEGO_OK;
}



    


StegoStatus lsbi_extract(const Bmp *bmp, size_t len, uint8_t *out) {
    // obtenemos cuales fueron invertidos
    uint8_t inverted[PATTERNS];
    for (int i = 0; i < PATTERNS; i++) {
        inverted[i] = get_lsb(bmp->data[i]);
    }

    // ahora vamos armando el mensaje en out (salteando rojos, igual que el embed)
    // j va afuera porque sigue avanzando de un byte de out al siguiente
    size_t j = PATTERNS;
    for (size_t i = 0; i < len; i++) {
        uint8_t out_byte = 0;
        for(size_t b=0; b < 8; b++, j++){
            j = skip_if_red(j);
            //junto el LSB de cada uno de los 8 bytes de la imagen que forman el byte de out
            uint8_t byte = bmp->data[j];
            uint8_t bit = get_lsb(byte) ^ inverted[pattern(byte)]; 
            out_byte = push_bit(out_byte, bit);
        }
        out[i] = out_byte;
    }

    return STEGO_OK;
}
