#include <string.h>
#include "structures.h"

/* Ensure number is >=0 and <=255 */
#define SATURATE( n )    ( ( n ) > 0 ? ( ( n ) < 255 ? ( n ) : 255 ) : 0 )

void cc ( const SubHeader1 * SH1, const PBlock * PB, ColorBuffer * CB )
{
    if ( SH1->n_comp > 1 ) {
        int           i, j;
        unsigned char y, cb, cr;
        signed char   rcb, rcr;
        long          r, g, b;
        long          offset;

        for ( i = 0; i < SH1->MCU_sy; i++ ) {           /* pixel rows */
            int ip_0           = i >> SH1->comp[0].VDIV;
            int ip_1           = i >> SH1->comp[1].VDIV;
            int ip_2           = i >> SH1->comp[2].VDIV;
            int inv_ndx_0      = SH1->comp[0].IDX + SH1->comp[0].HS * ( ip_0 >> 3 );
            int inv_ndx_1      = SH1->comp[1].IDX + SH1->comp[1].HS * ( ip_1 >> 3 );
            int inv_ndx_2      = SH1->comp[2].IDX + SH1->comp[2].HS * ( ip_2 >> 3 );
            int ip_0_lsbs      = ip_0 & 7;
            int ip_1_lsbs      = ip_1 & 7;
            int ip_2_lsbs      = ip_2 & 7;
            int i_times_MCU_sx = i * SH1->MCU_sx;

            for ( j = 0; j < SH1->MCU_sx; j++ ) {               /* pixel columns */
                int jp_0 = j >> SH1->comp[0].HDIV;
                int jp_1 = j >> SH1->comp[1].HDIV;
                int jp_2 = j >> SH1->comp[2].HDIV;

                y  = PB[inv_ndx_0 + ( jp_0 >> 3 )].block[ip_0_lsbs][jp_0 & 7];
                cb = PB[inv_ndx_1 + ( jp_1 >> 3 )].block[ip_1_lsbs][jp_1 & 7];
                cr = PB[inv_ndx_2 + ( jp_2 >> 3 )].block[ip_2_lsbs][jp_2 & 7];

                rcb = cb - 128;
                rcr = cr - 128;

                r = y + ( ( 359 * rcr ) >> 8 );
                g = y - ( ( 11 * rcb ) >> 5 ) - ( ( 183 * rcr ) >> 8 );
                b = y + ( ( 227 * rcb ) >> 7 );

                offset               = 3 * ( i_times_MCU_sx + j );
                CB->data[offset + 2] = SATURATE ( r );
                CB->data[offset + 1] = SATURATE ( g );
                CB->data[offset + 0] = SATURATE ( b );
                /* note that this is SunRaster color ordering */
            }
        }
    }
    else{
        memmove ( CB, PB, 64 );
    }
}
