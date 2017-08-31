#ifndef __EXTRACTEUR_H__
#define __EXTRACTEUR_H__
#include "jpeg_reader.h"
#include "bitstream.h"

/* Décode une valeur encodée en magnitude */
extern int16_t dec_magnitude(uint8_t magnitude, uint16_t val);
extern int16_t *dec_RLE(uint16_t symbole, uint8_t *magnitude, uint8_t *size, uint8_t totalecr);
extern struct mcu decoder(struct jpeg_desc *jdesc, struct bitstream *stream);
/* TODO module séparé */
extern uint8_t *nb_blocs(struct jpeg_desc *jdesc);
/* TODO structure spéciale pour bloc */
extern void free_mcu(struct mcu mcu);

#endif
