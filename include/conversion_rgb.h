#ifndef __CONVERSION_H__
#define __CONVERSION_H__
#include "jpeg_reader.h"
#include "structure.h"

extern uint32_t *conversion_rgb(struct fmcu fmcu, struct jpeg_desc *jdesc);
extern uint32_t *reordonne_pixel_fmcu(uint32_t* pixels, struct jpeg_desc* jdesc);
extern uint32_t *reordonne_pixel_image(uint32_t* pixels, struct jpeg_desc* jdesc);
extern uint32_t *generer_pixels(uint32_t *image, struct jpeg_desc *jdesc);
extern uint32_t *allouer_image(struct jpeg_desc *jdesc);

#endif
