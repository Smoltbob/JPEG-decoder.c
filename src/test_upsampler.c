#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "extracteur.h"
#include "traitement.h"
#include "upsampling.h"
#include "structure.h"

/*
affiche la premiere mcu decodee et traitee d'un fichier jpeg puis la meme mcu upsamplee

Nota: 1) on peut verifier que la mcu decodee est correcte grace a test_decoder
      2) il faut verifier "a la main" que l'upsampling est correcte
*/
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *filename = argv[1];
    if( access( filename, F_OK ) == -1 ) {
        fprintf(stderr, "Error : file %s does not exist\n", argv[1]);
        return EXIT_FAILURE;
    }
    /* On cree un jpeg_desc qui permettra de lire le fichier. */
    struct jpeg_desc *jdesc = read_jpeg(filename);
    /* On recupere le flux des donnees brutes a partir du descripteur. */
    struct bitstream *stream = get_bitstream(jdesc);
    /* Décodage de la premiere mcu a partir du flux */
    struct mcu mcu = decoder(jdesc, stream);
    /* traitement mathematique */
    traiter_mcu(&mcu, jdesc);
    affiche_mcu(mcu);
    /* upsampling */
    struct fmcu fmcu_up = upsampling(&mcu, jdesc);
    affiche_fmcu(fmcu_up, mcu.nbcomp);
    free_mcu(mcu);
    free_fmcu(fmcu_up);
    close_jpeg(jdesc);
    return EXIT_SUCCESS;
}
