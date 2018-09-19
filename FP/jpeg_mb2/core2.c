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



int main (void)
{
	
	start_stack_check();
 
  hw_declare_dmas(1);
	hw_dma_add(1, (int*)mb1_dma0_BASEADDR);
	int dma_num = 0;
#if HAS_DMA
  dma_num = 1;
#endif

  FCB *fcb_R = fifo_add(
			MB2_FIFO_WC_R,
			MB2_FIFO_RC_R,
			MB2_FIFO_RWC_R,
			MB2_FIFO_RRC_R,
			MB2_FIFO_BUFFER_R,
			MB2_FIFO_RBUFFER_R,
			2,
			sizeof(struct from1_to_2),
			1,1,
			dma_num,
			1,
			0,
			0);
      
   FCB *fcb_S = fifo_add(
			MB2_FIFO_WC_S,
			MB2_FIFO_RC_S,
			MB2_FIFO_RWC_S,
			MB2_FIFO_RRC_S,
			MB2_FIFO_BUFFER_S,
			MB2_FIFO_RBUFFER_S,
			2,
			sizeof(struct from2_to_3),
			1,1,
			dma_num,
			1,
			0,
			0);
  // Sync with the monitor.
	mk_mon_sync();
  TIME t1, t2, diff;
  t1 = hw_tifu_systimer_get();

	// Paint it our 'group' color so we can identify it.


   // VldHeader header;
	// Start decoding the JPEG.
    //init_header_vld ( &header, shared_mem, output_mem);

    //SubHeader2 sh2;
    struct from1_to_2 re_from1;
    struct from2_to_3 se_to3;
    while(1)
    {
        //FValue fv[10];
        //SubHeader1 sh1;
        //header_vld ( &header, &header, &(fv[0]), &sh1, &sh2  );       
        
        fifo_set_consumption_token_rate(fcb_R, 1);
		    while(!fifo_check_data(fcb_R));
		    volatile struct from1_to_2 *sp_R = fifo_claim_data(fcb_R);
		    fifo_pull(fcb_R);
 	      re_from1 = *sp_R;
		    fifo_release_space(fcb_R);
        
        //PBlock pbout[10];
        for ( int i = 0; i < 10 ; i++ )
        {
            FBlock fbout;
            iqzz( &(re_from1.fv[i]), &fbout);
            idct( &fbout, &(se_to3.pbout[i]));
        }
        
        se_to3.sh1=re_from1.sh1;
        se_to3.sh2=re_from1.sh2;
        
        fifo_set_production_token_rate(fcb_S, 1);
		    while(!fifo_check_space(fcb_S));
		    volatile struct from2_to_3 *sp_S = fifo_claim_space(fcb_S);
		    *sp_S = se_to3;
		    fifo_push(fcb_S);
        fifo_release_data(fcb_S);
        
        //ColorBuffer cbout;
        //cc( &sh1, &(pbout[0]), &cbout);       
        //raster( &sh2, &cbout );

        if ( re_from1.sh2.leftover == 0 ){
            break;
        }
    }


  t2 = hw_tifu_systimer_get();
  diff = t2-t1;
  mk_mon_debug_info(LO_64(diff));
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}
