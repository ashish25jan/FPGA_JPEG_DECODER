#ifndef __5KK03_H__
#define __5KK03_H__
#include <stdio.h>
#include "structures.h"

unsigned int FGETC ( JPGFile *fp );
int FSEEK ( JPGFile *fp, int offset, int start );
size_t FTELL ( JPGFile *fp );

void my_fprintf ( FILE *stream, char *d, ... ) __attribute__( ( format ( printf, 2, 3 ) ) );
#endif