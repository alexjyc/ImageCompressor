#ifndef IO_INCLUDED
#define IO_INCLUDED

#include <stdlib.h>
#include "pnm.h"
#include "a2methods.h"

#define T A2Methods_UArray2
#define T_Interface A2Methods_T

extern Pnm_ppm IO_read_plain_image(FILE *fp, T_Interface methods);

extern void IO_write_binary(FILE *fp, T image, T_Interface methods,
                            int blocksize, int codelength);

extern T IO_read_binary(FILE *fp, T_Interface methods, int blocksize, 
                        int codelength);

#undef T 
#undef T_Interface
#endif