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


volatile unsigned int *output_mem = (unsigned int*) shared_pt_REMOTEADDR;
volatile unsigned int *shared_mem = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024);
volatile unsigned char *remote_buffer = (unsigned int*) (shared_pt_REMOTEADDR + 4*1024*1024 + 512*1024);

VldHeader header;
TIME t1, t2, diff;
//int dma_num = 1;
int count=0;
struct vld3_to_1 from3_to_1;
int dma_num = 1;

 
int main (void)
{
  // Sync with the monitor.
  // Enable stack checking.
 
  start_stack_check();
  
  hw_declare_dmas(1);
  hw_dma_add(1, (int*)mb3_dma0_BASEADDR);

#if HAS_DMA
  dma_num = 1;
#endif

 //mk_mon_sync();

FCB *fcb1 = fifo_add(
      MB3_FIFO_WC1,
      MB3_FIFO_RC1,
      MB3_FIFO_RWC1,
      MB3_FIFO_RRC1,
      MB3_FIFO_BUFFER1,
      MB3_FIFO_RBUFFER1,
      1,
      sizeof(struct vld3_to_1),
      1,1,
      dma_num,
      1,
      0,
      0);
        
         //mk_mon_sync();
      
FCB *fcb2 = fifo_add(
      MB3_FIFO_WC2,
      MB3_FIFO_RC2,
      MB3_FIFO_RWC2,
      MB3_FIFO_RRC2,
      MB3_FIFO_BUFFER2,
      MB3_FIFO_RBUFFER2,
      1,
      sizeof(struct vld3_to_1),
      1,1,
      dma_num,
      1,
      0,
      0);
            
   mk_mon_sync();  
   

    t1 = hw_tifu_systimer_get();
   
    
    init_header_vld ( &header, shared_mem, output_mem);
    
    //struct vld3_to_1 from3_to_1;
    while(1)
    {
        
        
        //mk_mon_debug_info(0);
        header_vld ( &header, &header, &(from3_to_1.fv[0]), &from3_to_1.sh1, &from3_to_1.sh2  );
        
        if(count%2==0){
        
          
          fifo_set_production_token_rate(fcb1, 1);
        while(!fifo_check_space(fcb1));
        volatile struct vld3_to_1 *sp = fifo_claim_space(fcb1);
        *sp = from3_to_1;
        fifo_push(fcb1);
        fifo_release_data(fcb1);
        //mk_mon_debug_info(1);
          
            }
                      
        else{
          
          fifo_set_production_token_rate(fcb2, 1);
        while(!fifo_check_space(fcb2));
        volatile struct vld3_to_1 *sp = fifo_claim_space(fcb2);
        *sp = from3_to_1;
        fifo_push(fcb2);
        fifo_release_data(fcb2);
        //mk_mon_debug_info(2);
        
            }
        count=count+1;
        

        if ( from3_to_1.sh2.leftover == 0 ){
            break;
        }
    }

  t2 = hw_tifu_systimer_get();
  diff = t2-t1;
  double ET = diff/2;
  mk_mon_debug_info(ET);
  // Signal the monitor we are done.
  mk_mon_debug_tile_finished();
  return 0;
}
