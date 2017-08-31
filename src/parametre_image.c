#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "jpeg_reader.h"
#include <stdbool.h>
#include "parametre_image.h"

/*
calcul de l'echantillonage
*/
enum echantillonage echantillonage(struct jpeg_desc* jdesc)
{
    uint8_t nbcomp = get_nb_components(jdesc);
    if (nbcomp == 1) {
      return NO_ECH;
    }
    if (nbcomp == 3) {
      uint8_t YH = get_frame_component_sampling_factor(jdesc, DIR_H, 0);
      uint8_t YV = get_frame_component_sampling_factor(jdesc, DIR_V, 0);
      uint8_t CH = get_frame_component_sampling_factor(jdesc, DIR_H, 1);
      uint8_t CV = get_frame_component_sampling_factor(jdesc, DIR_V, 1);
      bool CbCr = (CH == get_frame_component_sampling_factor(jdesc, DIR_H, 2));
      CbCr &= (CV == get_frame_component_sampling_factor(jdesc, DIR_V, 2));
      if (! CbCr) {
          return ECH_NB;
      }
      uint8_t facth = YH/CH;
      uint8_t factv = YV/CV;
      if (facth == 2 && factv == 2) {
        return ECH_HV;
      } else if ((facth == 2) && (factv == 1)) {
        return ECH_H;
      } else if ((factv == 2) && (facth == 1)) {
        return ECH_V;
      } else if ((facth == 1) && (factv == 1)) {
        return NO_ECH;
      }
    }
  return ECH_NB;
}

/*
Retourne les vraies dimensions de l'image dans les 3 cas présentés dans le
sujet. TODO : calcul dans le cas général
*/
uint16_t cote_image(struct jpeg_desc* jdesc, enum direction DIR)
{
    uint16_t cote = get_image_size(jdesc, DIR);
    uint8_t cote_mcu = get_frame_component_sampling_factor(jdesc, DIR, 0);
    uint16_t fact = cote_mcu*8;
    cote = (cote % fact == 0) ? cote : (cote / fact+1) * fact;
    return cote;
}

/*
calcul du nombre de mcu
*/
uint32_t nombre_mcu(struct jpeg_desc* jdesc)
{
    uint16_t hauteur = cote_image(jdesc, DIR_V);
    uint16_t largeur = cote_image(jdesc, DIR_H);
    /* taille mcu en blocs */
    uint16_t hauteur_mcu = get_frame_component_sampling_factor(jdesc, DIR_V, 0);
    uint16_t largeur_mcu = get_frame_component_sampling_factor(jdesc, DIR_H, 0);
    /* calcul du nb de mcu */
    uint32_t nb_mcu = (hauteur*largeur)/(hauteur_mcu*largeur_mcu*64);
    return nb_mcu;
}
