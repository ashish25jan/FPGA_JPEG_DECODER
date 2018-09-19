#include "structures.h"
#include "stdarg.h"
#include "5kk03.h"

int vld_count = 0;

void my_fprintf ( FILE *stream, char *d, ... )
{
#ifdef PC
    va_list list;
    va_start ( list, d );
    vfprintf ( stream, d, list );
    va_end ( list );
#endif
}

unsigned int FGETC ( JPGFile *fp )
{
    unsigned int c = ( ( fp->data[fp->vld_count / 4] << ( 8 * ( 3 - ( fp->vld_count % 4 ) ) ) ) >> 24 ) & 0x00ff;
    fp->vld_count++;
    return c;
}

int FSEEK ( JPGFile *fp, int offset, int start )
{
    fp->vld_count += offset + ( start - start );    /* Just to use start... */
    return 0;
}

size_t FTELL ( JPGFile *fp )
{
    return fp->vld_count;
}
