#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>
#include <hw_tifu.h>
#include "structures.h"
#include "actors.h"\
#include <math.h>

volatile unsigned int *output_mem = (unsigned int*) shared_pt_REMOTEADDR;
volatile unsigned int *shared_mem = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024);
volatile unsigned char *remote_buffer = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024 + 512*1024);
TIME t1, t2 ;

VldHeader header;
SubHeader2 sh2;
FValue fv[10];
SubHeader1 sh1;
PBlock pbout[10];
FBlock fbout;
ColorBuffer cbout;
TIME diff;
double ET;


int p, q;


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
	
	// Sync with the monitor.
	mk_mon_sync();
	// Enable stack checking.
	//start_stack_check();
    t1 = hw_tifu_systimer_get();
	// Paint it our 'group' color so we can identify it.
	for ( int i = 566231 ; i < (786432);i++ ){
		output_mem[i] = 0x00000000;
	}


    VldHeader header;
	// Start decoding the JPEG.
    init_header_vld ( &header, shared_mem, output_mem);
    
	 
    int q = header.my_size;
	
	int max = ciel(q);
    
      while(1)
    {
        header_vld( &header, &header, &(fv[0]), &sh1, &sh2 );
		
		if(header.MCU_row < 4*max){
       
        for ( int i = 0; i < 10 ; i++ )
        {
            iqzz( &(fv[i]), &fbout);
            idct( &fbout, &(pbout[i]));
        }
        cc( &sh1, &(pbout[0]), &cbout);
        raster( &sh2, &cbout );
        
        } 
      
	  if ( header.MCU_row == (4*max)){
            break;
        }
        
     }
	//mk_mon_debug_info(p);
	t2 = hw_tifu_systimer_get();
	//mk_mon_debug_info(p);
    diff = t2-t1;
	
	ET = diff/120000;
	
	
	mk_mon_debug_info(ET);
	
	// Signal the monitor we are done.
	mk_mon_debug_tile_finished();
	return 0;
}