#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__
#include "jpeg_reader.h"
#include "parametre_image.h"

/* Structure qui contient un tableau de 64 valeurs pour chaque composante
 * (Y, Cb, Cr). Cb et Cr sont NULL si l'image est en noir et blanc. */
struct composantes {
    int16_t *luminance;
    int16_t *chb;
    int16_t *chr;
};

/* Structure qui contient des tableaux de composantes. Si une image est sous
 * échantillonée il peut y avoir plus d'un vecteur Y, Cb ou Cr par mcu.
 * Ex: YYCbCr. Cette structure contient toutes les composantes d'une mcu. */
struct tabcomposantes {
    int16_t **Y;
    int16_t **Cb;
    int16_t **Cr;
};

/* Décrit une mcu "brute" : la structure tabcomposantes contient les
 * vecteurs des différentes composantes. Le tableau t_compos indique la taille
 * de chaque tableau de tapcomposantes. ncomp indique le nombre de composantes
 * qui forment l'image (1 : n&b, 3: YCbCr) 
 */
struct mcu {
    struct tabcomposantes composantes;
    uint8_t *t_compos;
    uint8_t nbcomp;
};

/* Structure mcu formatée, utilisée pour réaliser l'upsampling */
struct fmcu {
    /* pointeur sur le premier "bloc" de composantes YCbCr */
    struct composantes* blocs;
    /* taille de la mcu */
    uint8_t largeur_mcu;
    uint8_t hauteur_mcu;
    /* nombre de bloc de la mcu */
    uint8_t nb_blocs;
    /* pour le cas général */
    uint8_t nb_blocs_filled;
};

extern void affiche_mcu(struct mcu mcu);
extern struct fmcu create_fmcu(struct jpeg_desc *jdesc);
extern void free_fmcu(struct fmcu fmcu);
extern void affiche_fmcu(struct fmcu fmcu, uint8_t nbcomp);
extern struct fmcu format_mcu(struct mcu* mcu, struct jpeg_desc* jdesc, enum echantillonage ech);

#endif
