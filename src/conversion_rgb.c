#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "clampe.h"
#include "parametre_image.h"
#include "structure.h"
#include "jpeg_reader.h"

uint8_t clampe(float val)
{
    val = roundf(val);
    val = (val > 255 ) ? 255 : val;
    val = (val < 0) ? 0 : val;
    return (uint8_t) val;
}

/*
   troncature de l'image, les dimensions sont calculees a l'interieur

Nota:
a appeler apres reordonne_pixel_image
*/
uint32_t *troncature(uint32_t *image, struct jpeg_desc *jdesc)
{
    /* dimensions de l'image */
    uint16_t largeur_bis = cote_image(jdesc, DIR_H);
    uint16_t largeur = get_image_size(jdesc, DIR_H);
    uint16_t hauteur = get_image_size(jdesc, DIR_V);
    uint32_t taille = hauteur*largeur;

    /* malloc du tableau de pixels reel (sans ajout de pixels) */
    uint32_t *real = malloc(taille*sizeof(uint32_t));
    for (uint16_t j=0; j<hauteur; j++) {
        for (uint16_t i=0; i<largeur; i++) {
            real[j*largeur+i] = image[j*largeur_bis+i];
        }
    }
    free(image);
    return real;
}

/*
    convertit les valeurs YCbCr en RGB,
    il y a autant de valeurs que de pixels dans la mcu puisqu'on
    a "mis a l'echelle" grace au sur-echantillonage.

    entree :
    matrice avec 3 niveaux d'indirection - c'est sense representer une mcu, 
    c'est-a-dire un tableau de taille <nb_blocs_dans_mcu> contenant des blocs, 
    c'est-a-dire un pointeur sur un tableau de trois pointeur contenant les 
    64 valeurs de Y ou Cb ou Cr

    sortie : un gros tableau avec la representation RGB de tous les pixels de la mcu

*/
uint32_t *conversion_rgb(struct fmcu fmcu, struct jpeg_desc *jdesc)
{
    /* taille est en fait le nombre de pixels de la mcu */
    uint8_t nb_blocs_dans_mcu = fmcu.nb_blocs;
    uint32_t taille = 64*nb_blocs_dans_mcu;
    uint32_t *pixels = malloc(taille*sizeof(uint32_t));
    struct composantes* blocs = fmcu.blocs;
    float rougef, vertf, bleuf;
    uint8_t rouge, bleu, vert;
    uint8_t nbcomp = get_nb_components(jdesc);

    for (uint32_t i=0; i<nb_blocs_dans_mcu; i++)
        /* on itere sur les blocs */
    {
        struct composantes cour = blocs[i];
        for (uint8_t j=0; j<64; j++) {
            /* on itere sur les 64 valeurs d'un bloc */
            if (nbcomp == 3) {
                /* si l'image est en couleur */
                rougef = cour.luminance[j] + 1.402 * (cour.chr[j] - 128);
                vertf = cour.luminance[j]-0.34414*(cour.chb[j]-128)-0.71414*(cour.chr[j]-128);
                bleuf = cour.luminance[j]+1.772*(cour.chb[j]-128);
            } else {
                /* image en noir et blanc */
                rougef = cour.luminance[j];
                vertf = rougef;
                bleuf = vertf;
            }
            rouge = clampe(rougef);
            vert = clampe(vertf);
            bleu = clampe(bleuf);
            pixels[64*i+j] = rouge << 16;
            pixels[64*i+j] |= vert << 8;
            pixels[64*i+j] |= bleu;
        }
    }
    return pixels;
}

/*
   reordonne les pixels au sein d'une mcu:
   on enumere les pixels sur une ligne complete (pouvant traverser plusieurs blocs avant
   la ligne suivante

   hauteur represente la hauteur en nb de bloc de la mcu
   largeur la largeur en nb de bloc de la mcu
   */
uint32_t *reordonne_pixel_fmcu(uint32_t* pixels, struct jpeg_desc* jdesc)
{
    uint16_t hauteur = get_frame_component_sampling_factor(jdesc, DIR_V, 0);
    uint16_t largeur = get_frame_component_sampling_factor(jdesc, DIR_H, 0);
    uint32_t taille = hauteur*largeur*64;
    uint32_t *copie = malloc(taille*sizeof(uint32_t));

    for (uint32_t j=0; j<hauteur; j++) {
        /*
           si on saute une ligne de bloc, on mange largeur*nb_pixels_blocs dans la copie
           idem dans pixel (commutatif) */
        uint32_t offset = largeur*64*j;
        for (uint32_t h=0; h<8; h++) {
            /*
               si on saute une ligne, on mange largeur*nb_pixels_en_ligne
               et on mange nb_pixels_en_ligne dans pixel */
            uint32_t offest = h*largeur*8;
            for (uint32_t i=0; i < largeur; i++) {
                /*
                   si on passe sur une ligne du bloc d'a cote, on mange nb_pixels_en_ligne
                   et on mange nb_pixels_blocs dans pixel */
                for (uint8_t k=0; k<8; k++) {
                    copie[offset+offest+k+i*8] = pixels[offset+h*8+i*64+k];
                }
            }
        }
    }
    return copie;
}

/*
   reordonne les pixels au sein de l'image:
   on enumere les pixels sur une ligne complete (pouvant traverser plusieurs mcu avant
   la ligne suivante

   hauteur represente la hauteur en mcu de l'image
   largeur la largeur en mcu de l'image

Nota:
1) on suppose que chaque mcu a ete ordonnee par reordonne_pixel_mcu ci-dessus
2) les dimensions de l'image sont donnees en nb de mcu !!!!
3) mais les dimensions de la mcu sont donnees en blocs
*/
uint32_t *reordonne_pixel_image(uint32_t* pixels, struct jpeg_desc* jdesc)
{

    uint16_t largeur = cote_image(jdesc, DIR_H);
    uint16_t hauteur = cote_image(jdesc, DIR_V);
    uint16_t hauteur_mcu = get_frame_component_sampling_factor(jdesc, DIR_V, 0);
    uint16_t largeur_mcu = get_frame_component_sampling_factor(jdesc, DIR_H, 0);

    uint32_t nb_de_pixels_dans_mcu = hauteur_mcu*largeur_mcu*64;
    uint32_t nb_pixels_en_ligne = largeur_mcu*8;
    uint32_t nb_pixels_en_hauteur = hauteur_mcu*8;
    hauteur = hauteur/nb_pixels_en_hauteur;
    largeur = largeur/nb_pixels_en_ligne;
    uint32_t taille = hauteur*largeur*nb_de_pixels_dans_mcu;
    uint32_t *copie = malloc(taille*sizeof(uint32_t));

    for (uint32_t j=0; j<hauteur; j++) {
        /*
           si on saute une ligne de mcu, on mange largeur*nb_pixels_mcu dans la copie
           idem dans pixel (commutatif) */
        uint32_t offset = largeur*nb_de_pixels_dans_mcu*j;
        for (uint32_t h=0; h<nb_pixels_en_hauteur; h++) {
            /*
               si on saute une ligne, on mange largeur*nb_pixels_en_ligne
               et on mange nb_pixels_en_ligne dans pixel */
            uint32_t offest = h*largeur*nb_pixels_en_ligne;
            for (uint32_t i=0; i < largeur; i++) {
                /*
                   si on passe sur une ligne de la mcu d'a cote, on mange nb_pixels_en_ligne
                   et on mange nb_pixels_mcu dans pixel */
                for (uint8_t k=0; k<nb_pixels_en_ligne; k++) {
                    copie[offset+offest+k+i*nb_pixels_en_ligne] = pixels[offset+h*nb_pixels_en_ligne+i*nb_de_pixels_dans_mcu+k];
                }
            }
        }
    }
    return copie;
}

uint32_t *generer_pixels(uint32_t *image, struct jpeg_desc *jdesc)
{
    uint32_t *image_bis = reordonne_pixel_image(image, jdesc);
    return troncature(image_bis, jdesc);
}

uint32_t *allouer_image(struct jpeg_desc *jdesc)
{
    uint16_t largeur = cote_image(jdesc, DIR_H);
    uint16_t hauteur = cote_image(jdesc, DIR_V);
    return malloc(hauteur*largeur*sizeof(uint32_t));
}
