/* lsbi.c - LSB Improved (Majeed y Sulaiman). Falta implementarlo. */
#include "steg.h"

/*
La idea es hacer un lsb, teniendo la cover image y la stego image
agrupar los bytes segun su 2do y 3er bit menos significativos (4 grupos: '00', '01', '10', '11')
fijarse por cada byte si el lsb cambio, si en un determinado grupo cambio mas de lo que se mantuvo -> lo invertimos en la cover image

hay que agregar al principio cuales van a estar invertidos y cuales no
*/

StegoStatus lsbi_embed(Bmp *bmp, const uint8_t *data, size_t len) {
}

StegoStatus lsbi_extract(const Bmp *bmp, size_t len, uint8_t *out) {
}
