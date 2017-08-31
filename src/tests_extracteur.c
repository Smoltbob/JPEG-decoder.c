#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "extracteur.h"
/* Module de test du décodage de valeurs encodées en RLE ou sous forme de
 * magnitudes */

int main(void)
{
    /* Test de dec_magnitude */
    assert(dec_magnitude(5, 0b00101) == -26);
    assert(dec_magnitude(5, 0b10001) == 17);
    assert(dec_magnitude(2, 0b00) == -3);
    assert(dec_magnitude(3, 0b111) == 7);
    assert(dec_magnitude(1, 0b1) == 1);
    printf("Tests magnitude : ok\n");

    /* Test de dec_RLE */
    uint8_t size = 0;
    uint8_t magnitude = 0;
    int16_t *res = dec_RLE(0x00, &magnitude, &size, 0);
    int16_t eob[63] = {0};
    assert(size == 63);
    for (uint16_t i = 0; i < size; i++) {
        assert(eob[i] == res[i]);
    }
    free(res);

    res = dec_RLE(0xF0, &magnitude, &size, 0);
    int16_t zrl[16];
    assert(size == 16);
    for (uint16_t i = 0; i < size; i++) {
        assert(zrl[i] == res[i]);
    }
    free(res);

    res = dec_RLE(0x25, &magnitude, &size, 0);
    assert(size == 2);
    assert(magnitude == 5);
    for (uint16_t i = 0; i < size; i++) {
        assert(res[i] == 0);
    }
    printf("Tests RLE : ok\n");
    free(res);
    return 1;
}
