#ifndef __TRAITEMENT_H__
#define __TRAITEMENT_H__
#include "jpeg_reader.h"
#include "structure.h"

extern void traiter_mcu(struct mcu* mcu, struct jpeg_desc *jdesc);
extern void iDCT_loeffler(int16_t *tab_frequences);
extern void iDCT(int16_t *tab_frequences);
extern void zigzag_inverse(int16_t *tab_frequences);
#endif
