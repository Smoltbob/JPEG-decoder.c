#ifndef __PARAMETRE_H__
#define __PARAMETRE_H__
#include "jpeg_reader.h"

enum echantillonage
{
    ECH_V,
    ECH_H,
    ECH_HV,
    /* pas d'echantillonage */
    NO_ECH,
    /* echantillonage non reconnu */
    ECH_NB
};

extern uint32_t nombre_mcu(struct jpeg_desc* jdesc);
extern uint16_t cote_image(struct jpeg_desc* jdesc, enum direction DIR);
enum echantillonage echantillonage(struct jpeg_desc *);

#endif
