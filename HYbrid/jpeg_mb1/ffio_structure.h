#include <comik.h>
#include <global_memmap.h>
#include <5kk03-utils.h>

#include "structures.h"
#include "actors.h"
#include <hw_dma.h>
#include <fifo.h>
#include <memmap.h>


typedef struct from1_to_2
{
  SubHeader1 sh1;
  SubHeader2 sh2;
  FValue fv[10];
};


typedef struct from2_to_3
{
  SubHeader1 sh1;
  SubHeader2 sh2;
  PBlock pbout[10];
};

typedef struct from2_to_4
{
  SubHeader1 sh1;
  SubHeader2 sh2;
  PBlock pbout[10];
  
};