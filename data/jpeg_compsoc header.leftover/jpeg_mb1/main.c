#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include <hw_tifu.h>
#include "structures.h"
#include "actors.h"
#include <math.h>

volatile unsigned int *output_mem = (unsigned int*) shared_pt_REMOTEADDR;
volatile unsigned int *shared_mem = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024);
volatile unsigned char *remote_buffer = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024 + 512*1024);



TIME diff;
double ET;
FValue fv[10];
SubHeader1 sh1;
int p, q, j, max1, max;
SubHeader2 sh2;

int ciel(int q){
	
	int x = q/10 ;
	if(x * 10 < q){
		x++;
	}
	else{
		x=x;
	}
    return x;
}

int main (void)
{
	TIME t1, t2 ;
	// Sync with the monitor.
	mk_mon_sync();
	// Enable stack checking.
	start_stack_check();
    t1 = hw_tifu_systimer_get();
	// Paint it our 'group' color so we can identify it.
	// Paint it our 'group' color so we can identify it.
	for ( int i = 0 ; i < (235930);i++ ){
		output_mem[i] = 0x00000000;
	}

	
    VldHeader header;
	// Start decoding the JPEG.
    init_header_vld ( &header, shared_mem, output_mem);

    SubHeader2 sh2;
	int q = header.leftover;
    int max = (q - ciel(8*q));
	
	do
    {
        if(q > max){
		
			header_vld ( &header, &header, &(fv[0]), &sh1, &sh2  );	
			
			q--;
		
		}
		
		
		if(q <= max){
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
		q--;
        }  
		
		if(sh2.leftover==0){
			break;
		}
    }while(q>0);
	//mk_mon_debug_info(p);
	t2 = hw_tifu_systimer_get();
	
	TIME diff = t2-t1;
	
	double ET = diff/60000;
	
	
	mk_mon_debug_info(ET);
	
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}