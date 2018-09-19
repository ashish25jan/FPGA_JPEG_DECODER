#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>

#include "structures.h"
#include "actors.h"
#include <hw_dma.h>
#include <fifo.h>
#include <memmap.h>


typedef struct vld3_to_1
{
  SubHeader1 sh1;
  SubHeader2 sh2;
  FValue fv[10];
  
};

