/**
 * FF_TYPES
 *
 * Definition of all pose types
 *
 * Copyright 2014 Eindhoven University of Technology
 */
#ifndef __FF_TYPES_H__
#define __FF_TYPES_H__

#include <mk_types.h>

#define WRAP_SET_MASK    0x80000000
#define WRAP_RST_MASK    0x00000000
#define WR_RD_MASK       0x7FFFFFFF

/**
 * enumerator for indicating allocated
 * block of memory.
 */
typedef enum
{
  NO_ALLOC      = 0,
  BUFFER_MEMORY = 1,
  WRITEC_MEMORY = 2,
  READC_MEMORY  = 4
} FifoMemory;

/**
 * FIFO control block
 */
struct FCB_s
{
  /** Bitmask to keep track of memory that is allocated. */
  FifoMemory allocated_memory;

  // c-heap admin
  /** The local buffer */
  volatile void* buffer;

  /** The local write counter */
  volatile int* writec;

  /** The local read counter */
  volatile int* readc;

  /** The remote buffer */
  volatile void* remote_buffer;

  /** The remote write counter */
  volatile int* remote_writec;

  /** The remote read counter */
  volatile int* remote_readc;

  // Fifo specification.
  /** Size of the fifo (in tokens) */
  int fifo_size;

  /** Size of the token (in bytes) */
  int token_size;

  /** Is/isn't a self fifo */
  int self_fifo;

  /** If write counter and buffer is in same memory */
  int same_memory;

  // pose extra admin
  /** Maximum production rate. (aka max claimed spaced) */
  int max_prod_token_rate;

  /** Maximum consumption rate. (aka max claimed data) */
  int max_cons_token_rate;

  /** Current production rate */
  int prod_token_rate;

  /** Current consumption rate */
  int cons_token_rate;

  /** prod ready */
  int prod_ready;

  /** cons ready */
  int cons_ready;

  /** space claimed up to now. */
  int claimed_space;

  /** data claimed up to now. */
  int claimed_data;

  /** pointer to DMA structure. */
  DMA* dma;

  /** Flag indicating to use dma */
  int use_dma;

  /** ID of last DMA write transaction */
  int write_id;
};

#endif // ifndef __FF_TYPES_H__
