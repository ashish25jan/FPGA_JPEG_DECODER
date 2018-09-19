#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include <hw_tifu.h>
#include "structures.h"
#include "actors.h"
#include <hw_dma.h>
#include <fifo.h>
#include <memmap.h>
#include <ffio_structure.h>

volatile unsigned int *output_mem = (unsigned int*) shared_pt_REMOTEADDR;
volatile unsigned int *shared_mem = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024);
volatile unsigned char *remote_buffer = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024 + 512*1024);

TIME t1, t2, diff;
//ColorBuffer cbout;
VldHeader header;

int main (void)
{
	start_stack_check();
  hw_declare_dmas(1);
	hw_dma_add(1, (int*)mb3_dma0_BASEADDR);
	int dma_num = 1;
#if HAS_DMA
  dma_num = 1;
#endif

	FCB *fcb = fifo_add(
			MB3_FIFO_WC,
			MB3_FIFO_RC,
			MB3_FIFO_RWC,
			MB3_FIFO_RRC,
			MB3_FIFO_BUFFER,
			MB3_FIFO_RBUFFER,
			2,
			sizeof(struct from3_to_2),
			1,1,
			dma_num,
			1,
			0,
			0);
  // Sync with the monitor.
	mk_mon_sync();
 
  

	// Paint it our 'group' color so we can identify it.

    //VldHeader header;
	// Start decoding the JPEG.
    t1 = hw_tifu_systimer_get();
    init_header_vld ( &header, shared_mem, output_mem);
    
    struct from3_to_2 core3_to_2;
    //SubHeader2 sh2;
    while(1)
    {
       
            
        //FValue fv[10];
        //SubHeader1 sh1;
        header_vld ( &header, &header, &(core3_to_2.fv[0]), &core3_to_2.sh1, &core3_to_2.sh2  );
        
        fifo_set_production_token_rate(fcb, 1);
		    while(!fifo_check_space(fcb));
		    volatile struct from3_to_2 *sp = fifo_claim_space(fcb);
		    *sp = core3_to_2;
		    fifo_push(fcb);
        fifo_release_data(fcb);
        
       /* PBlock pbout[10];
        for ( int i = 0; i < 10 ; i++ )
        {
            FBlock fbout;
            iqzz( &(fv[i]), &fbout);
            idct( &fbout, &(pbout[i]));
        }
        
        ColorBuffer cbout;
        cc( &sh1, &(pbout[0]), &cbout);
        
        raster( &sh2, &cbout ); */

        if ( core3_to_2.sh2.leftover == 0 ){
            break;
        }
    }


  t2 = hw_tifu_systimer_get();
  diff = (t2-t1)/2;
  mk_mon_debug_info(LO_64(diff));
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}
