#ifdef PC
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif
#include "structures.h"
#include <hw_dma.h>
#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>



typedef union RGBA
{
    unsigned int data;
    struct pixel
    {
        unsigned char b, g, r, a;
    } pixel;
}RGBA;

//volatile RGBA *mem = (mb1_cmemout0_BASEADDR + sizeof(int));

void raster ( const SubHeader2 * SH2, const ColorBuffer * CB )
{
    DMA dma = {0, mb1_dma0_BASEADDR };
    const int     goodrows    = SH2->goodrows;
    const int     goodcolumns = SH2->goodcolumns;
    const int     MCU_sx      = SH2->MCU_sx;
    const int     n_comp      = SH2->n_comp;
    int           i, j;
    const int     x_size = 1024;
    
    volatile RGBA *fb =  (RGBA *) SH2->fp.fb;
    volatile RGBA *mem = (mb4_cmemout0_BASEADDR );
    //volatile RGBA *mem = (RGBA *) SH2->fp.fb;
    for ( i = 0; i < goodrows; i++ ) {
        int row_offset = ( i + SH2->MCU_row * SH2->MCU_sy ) * x_size;
        for ( j = 0; j < goodcolumns; j++ ) {
            //int col_offset = j + ( SH2->MCU_column - 1 ) * SH2->MCU_sx;
            if ( n_comp == 3 ) {
                mem[j].pixel.r = CB->data[n_comp * i * MCU_sx + 2 + j * n_comp];
                mem[j].pixel.g = CB->data[n_comp * i * MCU_sx + 1 + j * n_comp];
                mem[j].pixel.b = CB->data[n_comp * i * MCU_sx + 0 + j * n_comp];
            }
            else if ( n_comp == 1 ) {
                mem[j].pixel.r = CB->data[n_comp * i * MCU_sx + 0 + j * n_comp];
                mem[j].pixel.g = CB->data[n_comp * i * MCU_sx + 0 + j * n_comp];
                //mem[row_offset + col_offset].pixel.b = CB->data[n_comp * i * MCU_sx + 0 + j * n_comp];
                mem[j].pixel.b = CB->data[n_comp * i * MCU_sx + 0 + j * n_comp];
            }
        }   
        volatile int col_offset = row_offset + ( SH2->MCU_column - 1 ) * SH2->MCU_sx;
        hw_dma_send(&fb[col_offset], mem, goodcolumns, &dma, NON_BLOCKING);
        while(hw_dma_status(&dma));  
                     
        
    }
    
}
