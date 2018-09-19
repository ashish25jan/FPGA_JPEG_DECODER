#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include "structures.h"
#include "actors.h"
#include <hw_dma.h>
#include <memmap.h>

volatile unsigned int *output_mem = (unsigned int*) shared_pt_REMOTEADDR;
volatile unsigned int *shared_mem = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024);
volatile unsigned char *remote_buffer = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024 + 512*1024);

int main (void)
{
	// Sync with the monitor.
	mk_mon_sync();
	// Enable stack checking.
	start_stack_check();

    TIME t1, t2, diff;
    t1 = hw_tifu_systimer_get(); 
    

    VldHeader header;
	// Start decoding the JPEG.
    init_header_vld ( &header, shared_mem, output_mem);
    int count=0;
    SubHeader2 sh2;

    
    while(1)
    {
        FValue fv[10];
        SubHeader1 sh1;
        if(count){ 
        header_vld ( &header, &header, &(fv[0]), &sh1, &sh2  );
        header_vld ( &header, &header, &(fv[0]), &sh1, &sh2  );
        header_vld ( &header, &header, &(fv[0]), &sh1, &sh2  );  
        }
        count=1;
        
        header_vld ( &header, &header, &(fv[0]), &sh1, &sh2  );
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
        //cycle++;
        if ( sh2.leftover == 0 ){
            break;
        }
    }
    //mk_mon_debug_info(cycle);
    t2 = hw_tifu_systimer_get();
    diff = t2-t1;
	double ET = diff/60000;
    mk_mon_debug_info(ET);
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}
