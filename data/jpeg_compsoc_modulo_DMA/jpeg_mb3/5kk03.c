#include "structures.h"
#include "stdarg.h"
#include "5kk03.h"
#include <hw_dma.h>
#include <global_memmap.h>


int vld_count = 0;
//volatile unsigned int *cmem_out = (unsigned int *) mb3_cmemout0_BASEADDR+sizeof(int)+2*sizeof(struct vld3_to_1);


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
    DMA dma = {0, mb3_dma0_BASEADDR };
    volatile unsigned int *cmem_out = (unsigned int *) (mb3_cmemout0_BASEADDR);
    
    //FrameBuffer_t *fb = (FrameBuffer_t *)shared_pt_REMOTEADDR;
    //FrameBuffer_t *data = (FrameBuffer_t *)mb1_cmemout0_BASEADDR; 
    //for (i = 0; i < 1024; i++) {
		//data[i] = FrameBuffer[i];
	//}
    volatile int ii=fp->vld_count / 4;
    /*int i=0;
    if((fp->vld_count)%4==0){
		hw_dma_receive(cmem_out, &(fp->data[i]), 1, &dma, NON_BLOCKING);
	  while(hw_dma_status(&dma));
    i++;}*/
    //mk_mon_debug_info(4);
    
    hw_dma_receive(cmem_out, &(fp->data[ii]), 1, &dma, NON_BLOCKING);
    while(hw_dma_status(&dma));
    unsigned int c = ( ( cmem_out[0] << ( 8 * ( 3 - ( fp->vld_count % 4 ) ) ) ) >> 24 ) & 0x00ff; 
    fp->vld_count++;
    //mk_mon_debug_info(5);
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
