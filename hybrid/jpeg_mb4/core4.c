#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>

#include "structures.h"
#include "actors.h"
#include <hw_dma.h>
#include <fifo.h>
#include <memmap.h>
#include "ffio_structure.h"


//volatile unsigned int *output_mem = (unsigned int*) shared_pt_REMOTEADDR;
//volatile unsigned int *shared_mem = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024);
//volatile unsigned char *remote_buffer = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024 + 512*1024);

//VldHeader header;
//TIME t1, t2, diff;
//int dma_num = 0;
//struct vld3_to_1 from3_to_1;
 
int main (void)
{
	// Sync with the monitor.
	// Enable stack checking.
  mk_mon_sync();
	start_stack_check();
/*  hw_declare_dmas(1);
	hw_dma_add(1, (int*)mb4_dma0_BASEADDR);
	//int dma_num = 1;
#if HAS_DMA
  dma_num = 1;
#endif

FCB *fcb = fifo_add(
			MB4_FIFO_WC,
			MB4_FIFO_RC,
			MB4_FIFO_RWC,
			MB4_FIFO_RRC,
			MB4_FIFO_BUFFER,
			MB4_FIFO_RBUFFER,
			2,
			sizeof(struct vld3_to_1),
			1,1,
			dma_num,
			1,
			0,
			0);
      
   mk_mon_sync();
      
 

    //
	// Start decoding the JPEG.
    //mk_mon_debug_info(10);
    t1 = hw_tifu_systimer_get();
    init_header_vld ( &header, shared_mem, output_mem);
    //mk_mon_debug_info(0);
    //SubHeader2 sh2;
    t1 = hw_tifu_systimer_get();
    
    struct vld3_to_1 from3_to_1;
    while(1)
    {
        //FValue fv[10];
        //SubHeader1 sh1;
        //mk_mon_debug_info(1);
        
        header_vld ( &header, &header, &(from3_to_1.fv[0]), &from3_to_1.sh1, &from3_to_1.sh2  );
        
        //mk_mon_debug_info(2);
        fifo_set_production_token_rate(fcb, 1);
		    while(!fifo_check_space(fcb));
		    volatile struct vld3_to_1 *sp = fifo_claim_space(fcb);
		    *sp = from3_to_1;
		    fifo_push(fcb);
		    fifo_release_data(fcb);
        //mk_mon_debug_info(3);
        PBlock pbout[10];
        for ( int i = 0; i < 10 ; i++ )
        {
            FBlock fbout;
            iqzz( &(fv[i]), &fbout);
            idct( &fbout, &(pbout[i]));
        }
        ColorBuffer cbout;
        cc( &sh1, &(pbout[0]), &cbout);
        raster( &sh2, &cbout );

        if ( from3_to_1.sh2.leftover == 0 ){
            break;
        }
    } 

  t2 = hw_tifu_systimer_get();
  diff = (t2-t1)/2;
  mk_mon_debug_info(LO_64(diff));    */
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}
