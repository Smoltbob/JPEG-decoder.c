#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "extracteur.h"
#include "traitement.h"
#include "conversion_rgb.h"
#include "ppm.h"
#include "parametre_image.h"
#include "upsampling.h"
#include "structure.h"
#include "securite.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
        return EXIT_FAILURE;
    }
    /* On recupere le nom du fichier JPEG sur la ligne de commande. */
    const char *filename = argv[1];
    if( access( filename, F_OK ) == -1 ) {
        fprintf(stderr, "Error : file %s does not exist\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (check_extension(filename, "jpg") == false && check_extension(filename, "jpeg") == false){
        fprintf(stderr, "Error : invalid extension. Expected : jpg ou jpeg.\n");
        return EXIT_FAILURE;
    }

    /* On cree un jpeg_desc qui permettra de lire ce fichier. */
    struct jpeg_desc *jdesc = read_jpeg(filename);
    /* On recupere le flux des donnees brutes a partir du descripteur. */
    struct bitstream *stream = get_bitstream(jdesc);

    /* taille image en pixels */
    uint32_t nb_mcu = nombre_mcu(jdesc);

    /* image finale */
    uint32_t *image = allouer_image(jdesc);
    for (uint32_t i = 0; i < nb_mcu; i++) {
        /* Décodage d'une mcu a partir du flux */
        struct mcu mcu = decoder(jdesc, stream);
        /* iQuant, Zigzag et iDCT */
        traiter_mcu(&mcu, jdesc);
        /* Conversion de la mcu en structure mcu suréchantilonnée */
        struct fmcu fmcu_up = upsampling(&mcu, jdesc);
        free_mcu(mcu);
        /* YCbCr -> RGB */
        uint32_t* pixels_mcu = conversion_rgb(fmcu_up, jdesc);
        free_fmcu(fmcu_up);
        uint32_t *pixels_ord = reordonne_pixel_fmcu(pixels_mcu, jdesc);
        free(pixels_mcu);
        for (uint32_t k=0; k<fmcu_up.nb_blocs*64; k++) {
            image[i*fmcu_up.nb_blocs*64+k] = pixels_ord[k];
        }
        free(pixels_ord);
    }
    /* Ordonancement des pixels et troncature */
    uint32_t *real = generer_pixels(image, jdesc);
    free(image);
    /* Génération du ppm */
    generer_ppm(real, jdesc);
    free(real);
    /* Nettoyage de printemps : close_jpeg ferme aussi le bitstream */
    close_jpeg(jdesc);
    return EXIT_SUCCESS;
}
