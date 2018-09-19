//#include <stdlib.h>
#include "structures.h"
#include "utilities.h"

/*----------------------------------------------------------------*/
/* find next marker of any type, returns it, positions just after */
/* EOF instead of marker if end of file met while searching ...	  */
/*----------------------------------------------------------------*/

extern char shared_mem[];

size_t      curr_pointer = 0;

unsigned int get_next_MK ( JPGFile * fp )
{
    unsigned int c;
    int          ffmet     = 0;
    int          locpassed = -1;

    while ( ( c = FGETC ( fp ) ) != (unsigned int) EOF ) {
        switch ( c )
        {
        case 0xFF:
            ffmet = 1;
            break;
        case 0x00:
            ffmet = 0;
            break;
        default:
            if ( locpassed > 1 ) {
                my_fprintf ( stderr, "NOTE: passed %d bytes\n", locpassed );
            }
            if ( ffmet ) {
                return 0xFF00 | c;
            }
            ffmet = 0;
            break;
        }
        locpassed++;
    }

    return (unsigned int) EOF;
}

unsigned int get_size ( void * fp )
{
    unsigned char aux;

    aux = FGETC ( fp );
    return ( aux << 8 ) | FGETC ( fp ); /* big endian */
}

void aborted_stream ( void * fp )
{
    my_fprintf ( stderr, "%zd:\tERROR:\tAbnormal end of decompression process!\n", FTELL ( fp ) );

//#ifdef DEBUG
//	exit(-1);
//#endif
}

/* Returns ceil(N/D). */
int ceil_div ( int N, int D )
{
    int i = N / D;

    if ( N > D * i ) {
        i++;
    }
    return i;
}

/* Returns floor(N/D). */
int floor_div ( int N, int D )
{
    int i = N / D;

    if ( N < D * i ) {
        i--;
    }
    return i;
}

void reset_prediction ( cd_t * comp )
{
    int i;

    for ( i = 0; i < 3; i++ ) {
        comp[i].PRED = 0;
    }
}

int reformat ( unsigned long S, int good )
{
    int St;

    if ( !good ) {
        return 0;
    }
    St = 1 << ( good - 1 );             /* 2^(good-1) */
    if ( S < (unsigned long) St ) {
        return S + 1 + ( ( -1 ) << good );
    }
    else{
        return S;
    }
}
