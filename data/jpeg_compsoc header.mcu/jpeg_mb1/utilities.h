#ifndef _JPEG_UTILITIES_H_INCLUDED
#define _JPEG_UTILITIES_H_INCLUDED
#ifdef PC
#include <stdio.h>
#include <stdlib.h>
#endif

#include "structures.h"

#define first_quad( c )     ( ( c ) >> 4 ) /* first 4 bits in file order */
#define second_quad( c )    ( ( c ) & 15 )

/*----------------------------------*/
/* JPEG format parsing markers here */
/*----------------------------------*/

#define SOI_MK    0xFFD8                /* start of image   */
#define APP_MK    0xFFE0                /* custom, up to FFEF */
#define COM_MK    0xFFFE                /* commment segment */
#define SOF_MK    0xFFC0                /* start of frame   */
#define SOS_MK    0xFFDA                /* start of scan    */
#define DHT_MK    0xFFC4                /* Huffman table    */
#define DQT_MK    0xFFDB                /* Quant. table     */
#define DRI_MK    0xFFDD                /* restart interval */
#define EOI_MK    0xFFD9                /* end of image     */
#define MK_MSK    0xFFF0

#define RST_MK( x )    ( ( 0xFFF8 & ( x ) ) == 0xFFD0 )

unsigned int get_next_MK ( JPGFile * fp );
unsigned int get_size ( void * fp );
void aborted_stream ( void * fp );
int ceil_div ( int N, int D );
int floor_div ( int N, int D );

void reset_prediction ( cd_t * comp );
int reformat ( unsigned long S, int good );

#endif                                                  /* _JPEG_UTILITIES_H_INCLUDED */
