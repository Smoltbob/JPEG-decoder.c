#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "conversion_rgb.h"
#include "structure.h"
#include "jpeg_reader.h"

/*
convertit un bloc 8x8 en representation RGB

Nota: le bloc pris comme exemple est le premier bloc de la mcu 2x1 de l'annexe A du sujet
*/
int main(void)
{
    struct fmcu fmcu;
    /* petites "astuces" pour initialiser nb_comp a 3 */
    struct jpeg_desc* jdesc = read_jpeg("images/horizontal.jpg");
    int16_t luminance[] = {0xa6, 0xa1, 0x9b, 0x9a, 0x9b, 0x9c, 0x97, 0x92,
                           0x9f, 0xa3, 0x9d, 0x8e, 0x89, 0x8f, 0x95, 0x94,
                           0xa5, 0x97, 0x96, 0xa1, 0x9e, 0x90, 0x90, 0x9e,
                           0xa7, 0x9b, 0x91, 0x91, 0x92, 0x91, 0x91, 0x94,
                           0xca, 0xda, 0xc8, 0x98, 0x85, 0x98, 0xa2, 0x96,
                           0xf0, 0xf7, 0xfb, 0xe8, 0xbd, 0x96, 0x90, 0x9d,
                           0xe9, 0xe0, 0xf1, 0xff, 0xef, 0xad, 0x8a, 0x90,
                           0xe7, 0xf2, 0xf1, 0xeb, 0xf7, 0xfb, 0xd0, 0x97
                          };
    int16_t ch_bleu[] = {0x75, 0x75, 0x75, 0x76, 0x76, 0x76, 0x76, 0x77,
                         0x75, 0x75, 0x75, 0x76, 0x76, 0x76, 0x77, 0x77,
                         0x75, 0x75, 0x76, 0x76, 0x76, 0x77, 0x77, 0x77,
                         0x76, 0x76, 0x76, 0x76, 0x77, 0x77, 0x77, 0x77,
                         0x76, 0x76, 0x76, 0x77, 0x77, 0x77, 0x78, 0x78,
                         0x76, 0x76, 0x77, 0x77, 0x77, 0x78, 0x78, 0x78,
                         0x76, 0x77, 0x77, 0x77, 0x78, 0x78, 0x78, 0x78,
                         0x77, 0x77, 0x77, 0x77, 0x78, 0x78, 0x78, 0x78
                        };
    int16_t ch_rouge[] = {0x8d, 0x8b, 0x88, 0x86, 0x85, 0x85, 0x86, 0x87,
                          0x8d, 0x8b, 0x88, 0x86, 0x85, 0x85, 0x86, 0x87,
                          0x8c, 0x8b, 0x88, 0x86, 0x85, 0x85, 0x86, 0x87,
                          0x8c, 0x8a, 0x88, 0x85, 0x84, 0x85, 0x86, 0x87,
                          0x8c, 0x8a, 0x87, 0x85, 0x84, 0x84, 0x85, 0x86,
                          0x8b, 0x8a, 0x87, 0x85, 0x84, 0x84, 0x85, 0x86,
                          0x8b, 0x89, 0x87, 0x84, 0x83, 0x84, 0x85, 0x86,
                          0x8b, 0x89, 0x87, 0x84, 0x83, 0x84, 0x85, 0x86
                         };
    struct composantes bloc;
    bloc.luminance = luminance;
    bloc.chb = ch_bleu;
    bloc.chr = ch_rouge;
    fmcu.blocs = &bloc;
    fmcu.nb_blocs = 1;
    fmcu.largeur_mcu = 1;
    fmcu.hauteur_mcu = 1;
    fmcu.nb_blocs_filled = 1;
    uint32_t *ptr = conversion_rgb(fmcu, jdesc);
    printf("**Premier bloc de la mcu 2x1 donnee en annexe A du sujet**\n");
    printf("\n");
    for (uint8_t i = 0; i<64; i++) {
        if (i%8 == 7) {
            printf("%x\n", ptr[i]);
        } else {
            printf("%x ", ptr[i]);
        }
    }
    printf("\n");
    printf("*Nota: il peut y avoir des erreurs d'arrondi liees au clampage*\n");
    free(ptr);
    close_jpeg(jdesc);
    return EXIT_SUCCESS;
}