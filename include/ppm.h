#ifndef PPM_H
#define PPM_H

#include <stdint.h>
#include <stdio.h>
#include "jpeg_reader.h"

/* Affiche l'en-tête en ascii */
void entete_ppm(FILE *fic, uint16_t hauteur, uint16_t largeur, uint16_t cmax);
/* Généère le code BINAIRE  du fichier ppm */
void cree_ppm(FILE *fic, uint32_t *image, uint16_t hauteur, uint16_t largeur);
/* Affiche l'en-tête en ascii */
void entete_pgm(FILE *fic, uint16_t hauteur, uint16_t largeur, uint16_t cmax);
/* Généère le code BINAIRE  du fichier pgm */
void cree_pgm(FILE *fic, uint32_t *image, uint16_t hauteur, uint16_t largeur);
/* Gère la génération d'image de A à Z */
void generer_ppm(uint32_t *image, struct jpeg_desc *jpeg);

#endif
