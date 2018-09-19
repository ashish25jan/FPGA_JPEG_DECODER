#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include <hw_tifu.h>
#include "structures.h"
#include "actors.h"
#include <hw_dma.h>
#include <fifo.h>
#include <memmap.h>
#include "ffio_structure.h"

/*struct vld_receive
{
  SubHeader1 sh1;
  SubHeader2 sh2;
  FValue fv[10];
  
};*/



int main (void)
{
	// Sync with the monitor.
	// Enable stack checking.
	start_stack_check();
  hw_declare_dmas(1);
	hw_dma_add(1, (int*)mb1_dma0_BASEADDR);
	int dma_num = 1;
#if HAS_DMA
  dma_num = 1;
#endif

FCB *fcb = fifo_add(
			MB1_FIFO_WC,
			MB1_FIFO_RC,
			MB1_FIFO_RWC,
			MB1_FIFO_RRC,
			MB1_FIFO_BUFFER,
			MB1_FIFO_RBUFFER,
			1,
			sizeof(struct vld3_to_1),
			1,1,
			dma_num,
			1,
			0,
			0);
     mk_mon_sync(); 
  
	// Paint it our 'group' color so we can identify it.
	//for ( int i = 0 ; i < (768*1024);i++ ){
	//	output_mem[i] = 0xFF1122FF;
	//}

    //SubHeader2 sh2;
    TIME t1, t2, diff;
    t1 = hw_tifu_systimer_get();
    struct vld3_to_1 receive_from_3;
    while(1)
    {
        //FValue fv[10];
        //SubHeader1 sh1;
        //header_vld ( &header, &header, &(from1_to_3.fv[0]), &from1_to_3.sh1, &from1_to_3.sh2  );
        //mk_mon_debug_info(4);
        
        fifo_set_consumption_token_rate(fcb, 1);
		    while(!fifo_check_data(fcb));
		    volatile struct vld3_to_1 *sp = fifo_claim_data(fcb);
		    fifo_pull(fcb);
 	      receive_from_3 = *sp;
		    fifo_release_space(fcb);
        //mk_mon_debug_info(4);
            
        PBlock pbout[10];
        for ( int i = 0; i < 10 ; i++ )
        {
            FBlock fbout;
            iqzz( &(receive_from_3.fv[i]), &fbout);
            idct( &fbout, &(pbout[i]));
        }
        ColorBuffer cbout;
        cc( &receive_from_3.sh1, &(pbout[0]), &cbout);
        raster( &receive_from_3.sh2, &cbout );

        if ( receive_from_3.sh2.leftover == 0 || receive_from_3.sh2.leftover == 1 ){
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



