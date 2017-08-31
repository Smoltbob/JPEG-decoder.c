#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "jpeg_reader.h"
#include "extracteur.h"
#include "structure.h"
#include "parametre_image.h"
/* Module de fonctions de manipulation des structures décrites dans le
 * projet. */

void allocate_blocs(struct fmcu* fmcu, uint8_t nbbloc_max, uint8_t nbcomp);

/* Affiche les composantes d'une MCU sous forme d'un carré 8x8 */
void affiche_mcu(struct mcu mcu)
{
    printf("***mcu***\n");
    int16_t **ptcompos[3] = {mcu.composantes.Y, mcu.composantes.Cb, mcu.composantes.Cr};
    for (uint8_t i = 0; i < mcu.nbcomp; i++) {
        printf("**composante %hhu**\n", i);
        for (uint8_t j = 0; j < mcu.t_compos[i]; j++) {
            printf("*bloc %hhu*\n", j);
            for (uint8_t p=0; p<64; p++) {
                if (p%8 == 7) {
                    printf("%x\n", (uint16_t)ptcompos[i][j][p]);
                } else {
                    printf("%x ", (uint16_t)ptcompos[i][j][p]);
                }
            }
        }
    }
}

/* Crée et remplit les attributs pour la structure de bloc. Attention : on ne
 *  fait pas de malloc ici, luminance, chb et chr doivent être alloués avant.
 */
struct composantes create_bloc(int16_t *luminance, int16_t *chb, int16_t *chr)
{
    struct composantes bloc;
    bloc.chb = chb;
    bloc.chr = chr;
    bloc.luminance = luminance;
    return bloc;
}

/* Libère une structure composante entière. */
void free_bloc(struct composantes *bloc)
{
    free(bloc->luminance);
    if (bloc->chb != NULL && bloc->chr != NULL) {
        free(bloc->chb);
        free(bloc->chr);
    }
}

/* Crée une mcu et calcule ses attributs numériques. On appelle allocate_blocs
 * pour allouer nb_blocs_dans_mcu blocs */
struct fmcu create_fmcu(struct jpeg_desc *jdesc)
{
    struct fmcu fmcu;
    /* Calcul des dimensions de la mcu */
    fmcu.hauteur_mcu = get_frame_component_sampling_factor(jdesc, DIR_V, 0);
    fmcu.largeur_mcu = get_frame_component_sampling_factor(jdesc, DIR_H, 0);
    uint8_t nbcomp = get_nb_components(jdesc);
    /* Calcul du nb de blocs */
    fmcu.nb_blocs = fmcu.hauteur_mcu*fmcu.largeur_mcu;
    /* Allocation memoire */
    fmcu.nb_blocs_filled = 0;
    allocate_blocs(&fmcu, fmcu.nb_blocs, nbcomp);
    return fmcu;
}

/* Alloue les blocs d'une fmcu */
void allocate_blocs(struct fmcu* fmcu, uint8_t nb_bloc, uint8_t nbcomp)
{
    /* tableau de blocs */
    struct composantes* blocs = malloc(nb_bloc*sizeof(struct composantes));
    fmcu->blocs = blocs;
    for (uint8_t i=0; i<nb_bloc; i++) {
        int16_t *luminance = malloc(64*sizeof(int16_t));
        int16_t *chb = NULL;
        int16_t *chr = NULL;
        /* si niveau de gris on n'alloue pas les chrominances */
        if (nbcomp == 3) {
            chb = calloc(64, sizeof(int16_t));
            chr = calloc(64, sizeof(int16_t));
        }
        /* on cree le bloc et on le met dans le tableau */
        blocs[i] = create_bloc(luminance, chb, chr);
        /* on incremente le compteur de blocs alloues */
        fmcu->nb_blocs_filled ++;
    }
}

/* Libere la memoire occupée par une mcu */
void free_fmcu(struct fmcu fmcu)
{
    struct composantes* blocs = fmcu.blocs;
    for (uint32_t i=0; i<fmcu.nb_blocs; i++) {
        free_bloc(&blocs[i]);
    }
    free(fmcu.blocs);
}

/* Convertit une mcu en fmcu (mcu formatée) utilisable pour le traitement
 * du sous-échantillonnage. */
void mcu_to_fmcu(struct fmcu* fmcu, struct mcu mcu, enum component COMP, enum echantillonage ech)
{
    struct composantes* blocs = fmcu->blocs;
    int16_t **ptcompos[3] = {mcu.composantes.Y, mcu.composantes.Cb, mcu.composantes.Cr};
    uint8_t i=0;
    struct composantes cour;
    for (uint8_t k=0; k<mcu.t_compos[COMP]; k+=1) {
        for (uint8_t p=0; p<64; p++) {
            switch (COMP) {
            case COMP_Y:
                cour = blocs[k];
                cour.luminance[p] = ptcompos[COMP][k][p];
                break;
            case COMP_Cb:
                i = k + (((ech == ECH_H) || (ech == ECH_V)) && k > 0);
                cour = blocs[i];
                cour.chb[p] = ptcompos[COMP][k][p];
                break;
            case COMP_Cr:
                i = k + (((ech == ECH_H) || (ech == ECH_V)) && k > 0);
                cour = blocs[i];
                cour.chr[p] = ptcompos[COMP][k][p];
                break;
            default:
                printf("Error : wrong component in mcu_to_fcmu\n");
                exit(1);
            }
        }
    }
}

/* remplit les blocs d'une mcu a partir d'un groupe */
struct fmcu format_mcu(struct mcu* mcu, struct jpeg_desc* jdesc, enum echantillonage ech)
{
    struct fmcu fmcu = create_fmcu(jdesc);
    /* remplissage des blocs a partir du groupe*/

    /* remplissage des blocs */
    mcu_to_fmcu(&fmcu, *mcu, (enum component)COMP_Y, ech);
    if (mcu->nbcomp == 3) {
        mcu_to_fmcu(&fmcu, *mcu, (enum component)COMP_Cb, ech);
        mcu_to_fmcu(&fmcu, *mcu, (enum component)COMP_Cr, ech);
    }
    return fmcu;
}

#define affiche_bloc(bloc, bloc_field)  \
    do {                                    \
        printf("***composante \n");           \
        for (uint8_t p=0; p<64; p++)          \
        {                                     \
            printf("%x%s", bloc.bloc_field[p], p%8 == 7 ? "\n" : " "); \
        }                                     \
    } while (0);

void affiche_fmcu(struct fmcu fmcu, uint8_t nbcomp)
{
    printf("***fmcu***\n");
    uint8_t lim = fmcu.nb_blocs;
    struct composantes *blocs = fmcu.blocs;
    for (uint8_t i=0; i<lim; i++) {
        printf("**bloc %hhu**\n", i);
        struct composantes cour = blocs[i];
        affiche_bloc(cour, luminance);
        if (nbcomp == 3) {
            affiche_bloc(cour, chb);
            affiche_bloc(cour, chr);
        }
    }
}
