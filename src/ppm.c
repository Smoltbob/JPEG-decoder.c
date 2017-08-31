/* Ce module gère la création d'images ppm */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ppm.h"

/* Affiche l'en-tête du fichier ppm, format P6 (binaire) en utilisant en
 * entrée une structure qui décrit les caractérisitiques du fichier */
void entete_ppm(FILE *fic, uint16_t hauteur, uint16_t largeur, uint16_t cmax)
{
    fprintf(fic, "P6\n");
    fprintf(fic, "%hu %hu\n", largeur, hauteur);
    fprintf(fic, "%i\n", cmax);
}

/* Crée un fichier ppm à partir d'une structure décrivant les caractérisitiques
 * et d'un tableau de pixels RGB décrits par un des entiers 32 bits */
void cree_ppm(FILE *fic, uint32_t *image, uint16_t hauteur, uint16_t largeur)
{
    /* Copie locale du pointeur sur le tableau */
    uint32_t *i32pixel = image;
    /* Génération de l'en-tête apropriée */
    static int8_t tete = 0;
    if (tete == 0) {
        entete_ppm(fic, hauteur, largeur, 255);
        tete = 1;
    }
    /* On recopie chaque pixel du tableau */
    for(uint32_t j = 0; j < hauteur*largeur; j++) {
        /* Extraction des coordonnées RGB à partir de l'entier */
        uint8_t R = (*i32pixel >> 4*4) & 0xFF;
        uint8_t G = (*i32pixel >> 2*4) & 0xFF;
        uint8_t B = *i32pixel & 0xFF;
        /* On place les valeurs dans un tableau et on les écrit en même
         * temps */
        uint8_t couleurs[] = {R, G, B};
        fwrite(couleurs, 1, 3, fic);
        i32pixel++;
    }
}

/* Affiche l'en-tête du fichier pgm, format P5 (binaire) en utilisant en
 * entrée une structure qui décrit les caractérisitiques du fichier */
void entete_pgm(FILE *fic, uint16_t hauteur, uint16_t largeur, uint16_t cmax)
{
    fprintf(fic, "P5\n");
    fprintf(fic, "%hu %hu\n", largeur, hauteur);
    fprintf(fic, "%i\n", cmax);
}

/* Crée un fichier pgm à partir d'une structure décrivant les caractérisitiques
 * et d'un tableau de pixels n&b décrits par un des entiers 32 bits */
void cree_pgm(FILE *fic, uint32_t *image, uint16_t hauteur, uint16_t largeur)
{
    /* Copie locale du pointeur sur le tableau */
    uint32_t *i32pixel = image;
    /* Génération de l'en-tête apropriée */
    entete_pgm(fic, hauteur, largeur, 255);
    /* On recopie chaque pixel du tableau */
    for(uint32_t j = 0; j < hauteur * largeur; j++) {
        /* Extraction des coordonnées RGB à partir de l'entier */
        uint8_t B = *i32pixel & 0xFF;
        uint8_t couleurs[] = {B};
        fwrite(couleurs, 1, 1, fic);
        i32pixel++;
    }
}

/* Remplace l'extension d'un nom de fichier */
char *change_extension(char *name, char *extension)
{
    int8_t idx = strlen(name) - 1;
    int8_t taille_ext = strlen(extension);
    /* On lit la chaîne depuis la fin pour chercher le premier point
     * (46 en ascii) */
    while (idx >= 0 && name[idx] != 46) {
        idx --;
    }
    /* idx = position du point et on veut pointer sur le char suivant */
    idx ++;
    /* Création du nouveau nom, avec suffisament de place pour l'extension */
    char *outname = malloc((idx + taille_ext + 1)*sizeof(char));
    strncpy(outname, name, idx);
    for (uint8_t i = 0; i < taille_ext; i++) {
        outname[idx + i] = extension[i];
    }
    outname[idx + taille_ext] = '\0';
    return outname;
}

/* Gère la génération d'image ppm (couleur) ou pgm (n&b) de A à Z */
void generer_ppm(uint32_t *image, struct jpeg_desc *jdesc)
{
    /* Récupération du nom du fichier */
    char *filename = get_filename(jdesc);
    uint8_t nbcomp = get_nb_components(jdesc);
    uint16_t hauteur = get_image_size(jdesc, DIR_V);
    uint16_t largeur = get_image_size(jdesc, DIR_H);
    if (nbcomp == 1) {
        /* Transformation du nom du fichier en *.pgm */
        char *outname = change_extension(filename, "pgm");
        /* Génération de l'image */
        FILE *fic = fopen(outname, "w");
        cree_pgm(fic, image, hauteur, largeur);
        fclose(fic);
        free(outname);
    } else {
        /* Transformation du nom du fichier en *.ppm */
        char *outname = change_extension(filename, "ppm");
        /* Génération de l'image */
        FILE *fic = fopen(outname, "w");
        cree_ppm(fic, image, hauteur, largeur);
        fclose(fic);
        free(outname);
    }
}
