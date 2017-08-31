#include <stdlib.h>
#include <stdint.h>
#include "structure.h"
#include "parametre_image.h"
#include "jpeg_reader.h"

/* Test des structures
   Nota: l'affichage se fait en hexa !!!
*/
int main(void)
{
    struct jpeg_desc *jdesc = read_jpeg("images/horizontal.jpg");
    /* creation d'une struct mcu */
    int16_t Y0[64];
    int16_t Y1[64];
    int16_t Cb0[64];
    int16_t Cr0[64];
    for (uint8_t i=0; i<64; i++) {
        Y0[i] = 2*i;
        Y1[i] = 2*i+1;
        Cb0[i] = 70;
        Cr0[i] = 80;
    }
    struct mcu mcu;
    int16_t *luminance[] = {Y0, Y1};
    int16_t *chb[] = {Cb0};
    int16_t *chr[] = {Cr0};
    struct tabcomposantes tabcomp;
    tabcomp.Y = luminance;
    tabcomp.Cb = chb;
    tabcomp.Cr = chr;
    mcu.composantes = tabcomp;
    uint8_t tab[] = {2, 1, 1};
    mcu.t_compos = tab;
    mcu.nbcomp = 3;
    /* affichage de la mcu */
    affiche_mcu(mcu);
    /* creation d'une struct fmcu a partir de la struct mcu */
    struct fmcu fmcu = format_mcu(&mcu, jdesc, ECH_H);
    affiche_fmcu(fmcu, 3);
    free_fmcu(fmcu);
    close_jpeg(jdesc);
    return EXIT_SUCCESS;
}
