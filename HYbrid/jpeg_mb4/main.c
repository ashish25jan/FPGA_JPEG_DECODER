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
ColorBuffer cbout;

int main (void)
{
	start_stack_check();
  hw_declare_dmas(1);
	hw_dma_add(1, (int*)mb1_dma0_BASEADDR);
	int dma_num = 0;
#if HAS_DMA
  dma_num = 1;
#endif

    t1 = hw_tifu_systimer_get();
	
	FCB *fcb = fifo_add(
			MB4_FIFO_WC,
			MB4_FIFO_RC,
			MB4_FIFO_RWC,
			MB4_FIFO_RRC,
			MB4_FIFO_BUFFER,
			MB4_FIFO_RBUFFER,
			2,
			sizeof(struct from2_to_4),
			1,1,
			dma_num,
			1,
			0,
			0);
  // Sync with the monitor.
	mk_mon_sync();
 
  

	// Paint it our 'group' color so we can identify it.

  struct from2_to_4 re_from2;
   
    while(1)
    {
        fifo_set_consumption_token_rate(fcb, 1);
		    while(!fifo_check_data(fcb));
		    volatile struct from2_to_4 *sp = fifo_claim_data(fcb);
		    fifo_pull(fcb);
 	      re_from2 = *sp;
		    fifo_release_space(fcb);
        
        
        

        cc( &re_from2.sh1, &(re_from2.pbout[0]), &cbout);       
        raster( &re_from2.sh2, &cbout );

        if ( re_from2.sh2.leftover == 0 ){
            break;
        }
    }
      


  t2 = hw_tifu_systimer_get();
  diff = (t2-t1)/2;
  double ET = diff/120000;
  mk_mon_debug_info(ET);
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}
