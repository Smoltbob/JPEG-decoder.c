#ifndef __UPSAMPLING_H__
#define __UPSAMPLING_H__
#include "jpeg_reader.h"

extern struct fmcu upsampling_horizontal(struct fmcu fmcu, struct jpeg_desc* jdesc);
extern struct fmcu upsampling_vertical(struct fmcu fmcu, struct jpeg_desc* jdesc);
extern struct fmcu upsampling_horizontal_vertical(struct fmcu fmcu, struct jpeg_desc* jdsec);
extern struct fmcu upsampling(struct mcu* mcu, struct jpeg_desc* jdesc);

#endif
