#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "structure.h"
#include "parametre_image.h"

void recopie_horizontale(struct composantes* blocs, struct composantes* blocs_bis, uint8_t i, uint8_t j, uint8_t k, uint8_t p)
{
    /*
       etape 1:
       dupliquer la premiere moitie de la demi ligne du bloc mcu[z] dans mcu_bis[i]
       */
    /* chrominance bleu */
    blocs_bis[i].chb[p+j*8] = blocs[i].chb[k+j*8];
    blocs_bis[i].chb[p+1+j*8] = blocs[i].chb[k+j*8];
    /* chrominance rouge */
    blocs_bis[i].chr[p+j*8] = blocs[i].chr[k+j*8];
    blocs_bis[i].chr[p+1+j*8] = blocs[i].chr[k+j*8];
    /*
       etape 2:
       dupliquer la deuxieme moitie dans mcu_bis[i+1]
       */
    /* chrominance bleu */
    blocs_bis[i+1].chb[p+j*8] = blocs[i].chb[k+4+j*8];
    blocs_bis[i+1].chb[p+1+j*8] = blocs[i].chb[k+4+j*8];
    /* chrominance rouge */
    blocs_bis[i+1].chr[p+j*8] = blocs[i].chr[k+4+j*8];
    blocs_bis[i+1].chr[p+1+j*8] = blocs[i].chr[k+4+j*8];
    /* on n'oublie pas de recopier la luminance */
    blocs_bis[i].luminance[k+j*8] = blocs[i].luminance[k+j*8];
    blocs_bis[i].luminance[k+4+j*8] = blocs[i].luminance[k+4+j*8];
    blocs_bis[i+1].luminance[k+j*8] = blocs[i+1].luminance[k+j*8];
    blocs_bis[i+1].luminance[k+4+j*8] = blocs[i+1].luminance[k+4+j*8];
}

void recopie_verticale(struct composantes* blocs, struct composantes* blocs_bis, uint8_t i, uint8_t j, uint8_t k, uint8_t p, uint8_t largeur_mcu)
{
    /*
       etape 1:
       dupliquer la premiere moitie de la demi ligne du bloc mcu[z] dans mcu_bis[i]
       */
    /* chrominance bleu */
    blocs_bis[i].chb[p*8+j] = blocs[i].chb[k*8+j];
    blocs_bis[i].chb[p*8+8+j] = blocs[i].chb[k*8+j];
    /* chrominance rouge */
    blocs_bis[i].chr[p*8+j] = blocs[i].chr[k*8+j];
    blocs_bis[i].chr[p*8+8+j] = blocs[i].chr[k*8+j];
    /*
       etape 2:
       dupliquer la deuxieme moitie dans mcu_bis[i+1]
       */
    /* chrominance bleu */
    blocs_bis[i+largeur_mcu].chb[p*8+j] = blocs[i].chb[k*8+4*8+j];
    blocs_bis[i+largeur_mcu].chb[p*8+8+j] = blocs[i].chb[k*8+4*8+j];
    /* chrominance rouge */
    blocs_bis[i+largeur_mcu].chr[p*8+j] = blocs[i].chr[k*8+4*8+j];
    blocs_bis[i+largeur_mcu].chr[p*8+8+j] = blocs[i].chr[k*8+4*8+j];
    /* on n'oublie pas de recopier la luminance */
    blocs_bis[i].luminance[k+j*8] = blocs[i].luminance[k+j*8];
    blocs_bis[i].luminance[k+4+j*8] = blocs[i].luminance[k+4+j*8];
    blocs_bis[i+largeur_mcu].luminance[k+j*8] = blocs[i+largeur_mcu].luminance[k+j*8];
    blocs_bis[i+largeur_mcu].luminance[k+4+j*8] = blocs[i+largeur_mcu].luminance[k+4+j*8];
}

/*
   tableau mcu de pointeurs sur les blocs (= tableau de pointeur sur les composantes)
   puis malloc de <nb_de_pixels_dans_mcu> et on remplit (-> upsampling)

Nota:
1) le tableau mcu est construit en iterant sur les blocs
malloc de <nb_blocs_dans_mcu> et on remplit avec les pointeurs
2) les blocs Cb et Cr sont presents dans un bloc sur deux
*/
struct fmcu upsampling_simple(struct fmcu fmcu, struct jpeg_desc* jdesc, enum echantillonage ECH)
{
    uint8_t sup;
    if (ECH == ECH_V) {
        sup = fmcu.largeur_mcu;
    } else {
        sup = fmcu.nb_blocs;
    }
    /* on remalloc la mcu complete parce que c'est vachement plus simple */
    struct fmcu fmcu_bis = create_fmcu(jdesc);
    struct composantes* blocs = fmcu.blocs;
    struct composantes* blocs_bis = fmcu_bis.blocs;
    /* on itere sur les blocs a remplir de mcu_bis */
    uint32_t i=0;
    uint32_t tmp=0;
    while (i<fmcu.nb_blocs) {
        for (uint8_t j=0; j<8; j++) {
            /* pour toutes les lignes du bloc */
            int8_t k = -1;
            for (uint8_t p=0; p<8; p+=2) {
                k += 1;
                if (ECH == ECH_H) {
                    recopie_horizontale(blocs, blocs_bis, i, j, k, p);
                } else {
                    recopie_verticale(blocs, blocs_bis, i, j, k, p, fmcu.largeur_mcu);
                }
            }
        }
        i+=(ECH+1);
        tmp = i/fmcu.largeur_mcu;
        i += ((ECH == ECH_V) && ((tmp&1) == 1))?fmcu.largeur_mcu:0;
    }
    free_fmcu(fmcu);
    return fmcu_bis;
}

struct fmcu upsampling_horizontal_vertical(struct fmcu fmcu, struct jpeg_desc* jdesc)
{
    struct fmcu fmcu_uph = upsampling_simple(fmcu, jdesc, (enum echantillonage)ECH_H);
    struct fmcu fmcu_upv = upsampling_simple(fmcu_uph, jdesc, (enum echantillonage)ECH_V);
    return fmcu_upv;
}

struct fmcu upsampling(struct mcu* mcu, struct jpeg_desc* jdesc)
{
    enum echantillonage ech = echantillonage(jdesc);
    struct fmcu fmcu = format_mcu(mcu, jdesc, ech);
    if (mcu->nbcomp == 1 || ech == NO_ECH)
    {
      return fmcu;
    }
    if (ech != ECH_NB) {
        struct fmcu fmcu_up;
        if (ech == ECH_H || ech == ECH_V) {
            fmcu_up = upsampling_simple(fmcu, jdesc, (enum echantillonage)ech);
        } else {
            fmcu_up = upsampling_horizontal_vertical(fmcu, jdesc);
        }
        return fmcu_up;
    }
    fprintf(stderr, "Error : sample not handled\n");
    exit(1);
}
