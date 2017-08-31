#include <stdlib.h>
#include <stdio.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "extracteur.h"
#include "traitement.h"
#include "conversion_rgb.h"
#include "ppm.h"
#include "parametre_image.h"
#include "upsampling.h"
#include "structure.h"


int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
        return EXIT_FAILURE;
    }
    /* On recupere le nom du fichier JPEG sur la ligne de commande. */
    const char *filename = argv[1];
    /* On cree un jpeg_desc qui permettra de lire ce fichier. */
    struct jpeg_desc *jdesc = read_jpeg(filename);
    /* On recupere le flux des donnees brutes a partir du descripteur. */
    struct bitstream *stream = get_bitstream(jdesc);

    /* taille image en pixels */
    uint32_t nb_mcu = nombre_mcu(jdesc);

    for (uint32_t i = 0; i < nb_mcu; i++) {
        /* Décodage d'une mcu */
        struct mcu mcu = decoder(jdesc, stream);

        int16_t **ptcompos[3] = {mcu.composantes.Y, mcu.composantes.Cb, mcu.composantes.Cr};
        for (uint8_t i = 0; i < mcu.nbcomp; i++) {
            for (uint8_t j = 0; j < mcu.t_compos[i]; j++) {
                printf("[  bloc] ");
                for (uint8_t k = 0; k < 64; k++) {
                    printf("%x ", (uint16_t)ptcompos[i][j][k]);
                }
                printf("\n");
            }
        }
        free_mcu(mcu);
    }
    close_jpeg(jdesc);
    return EXIT_SUCCESS;
}
