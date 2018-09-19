/**
 * OS_FIFO
 *
 *
 * Copyright 2012 Delft University of Technology
 */

#include <comik.h>
#include <hw_dma.h>
#include <fifo.h>
#include "fifo_types.h"

/*
 * Local Helper Functions
 */
static inline void memcpy_int(volatile int* dst, volatile int* src, int w_cnt)
{
  int i;

  if (dst == src)
  {
    return;
  }
  for (i = 0; i < w_cnt; i++)
  {
    dst[i] = ((volatile int*)src)[i];
  }
}

static inline int calc_space(FCB* fifo)
{
  int rd_mask = *(fifo->readc) & WRAP_SET_MASK;
  int wr_mask = *(fifo->writec) & WRAP_SET_MASK;
  int rd_idx  = *(fifo->readc) & WR_RD_MASK;
  int wr_idx  = *(fifo->writec) & WR_RD_MASK;

  if (rd_mask != wr_mask)
  {
    wr_idx += fifo->fifo_size;
  }

  return (fifo->fifo_size + rd_idx) - wr_idx;
}

static inline int calc_data(FCB* fifo)
{
  int rd_mask = *(fifo->readc) & WRAP_SET_MASK;
  int wr_mask = *(fifo->writec) & WRAP_SET_MASK;
  int rd_idx  = *(fifo->readc) & WR_RD_MASK;
  int wr_idx  = *(fifo->writec) & WR_RD_MASK;

  if (rd_mask != wr_mask)
  {
    wr_idx += fifo->fifo_size;
  }

  return wr_idx - rd_idx;
}

static inline void* calc_buffer_write_pointer(FCB* fifo)
{
  int index = fifo->claimed_space;

  if (fifo->remote_buffer == NULL)
  {
    index += (*(fifo->writec) & WR_RD_MASK);
    if (index >= fifo->fifo_size)
    {
      index -= fifo->fifo_size;
    }
  }
  return (void*)((void*)fifo->buffer + index * fifo->token_size);
}

static inline void* calc_buffer_read_pointer(FCB* fifo)
{
  int index = fifo->claimed_data;

  if (fifo->remote_buffer == NULL)
  {
    index += (*(fifo->readc) & WR_RD_MASK);
    if (index >= fifo->fifo_size)
    {
      index -= fifo->fifo_size;
    }
  }

  return (void*)((void*)fifo->buffer + index * fifo->token_size);
}

static inline void* calc_remote_buffer_write_pointer(FCB* fifo)
{
  return (void*)((void*)fifo->remote_buffer +
                 (*(fifo->writec) & WR_RD_MASK) * fifo->token_size);
}

static inline void* calc_remote_buffer_read_pointer(FCB* fifo)
{
  return (void*)((void*)fifo->remote_buffer +
                 (*(fifo->readc) & WR_RD_MASK) * fifo->token_size);
}

static inline void increment_local_readc(FCB* fifo)
{
  int rd_idx  = *(fifo->readc) & WR_RD_MASK;
  int rd_mask = *(fifo->readc) & WRAP_SET_MASK;
  int readc   = rd_idx + fifo->cons_token_rate;

  fifo->claimed_data -= fifo->cons_token_rate;

  if (readc >= fifo->fifo_size)
  {
    readc  -= fifo->fifo_size;
    rd_mask = rd_mask ^ WRAP_SET_MASK;
  }

  readc |= rd_mask;
  *(fifo->readc) = readc;
}

static inline int pull_remote_readc(FCB* fifo)
{
  if (fifo->remote_readc == NULL)
  {
    return 0;
  }

  if (fifo->use_dma)
  {
    hw_dma_receive(fifo->readc, fifo->remote_readc, 1, fifo->dma, BLOCKING);
  } else
  {
    memcpy_int(fifo->readc, fifo->remote_readc, 1);
  }
  return 1;
}

static inline int push_local_readc(FCB* fifo)
{
  if (fifo->remote_readc == NULL)
  {
    return 0;
  }

  if (fifo->use_dma)
  {
    fifo->write_id = hw_dma_send(fifo->remote_readc, fifo->readc, 1, fifo->dma, NON_BLOCKING);
  } else
  {
    memcpy_int(fifo->remote_readc, fifo->readc, 1);
  }
  return 1;
}

static inline void increment_local_writec(FCB* fifo)
{
  int wr_idx  = *(fifo->writec) & WR_RD_MASK;
  int wr_mask = *(fifo->writec) & WRAP_SET_MASK;
  int writec  = wr_idx + fifo->prod_token_rate;

  fifo->claimed_space -= fifo->prod_token_rate;

  if (writec >= fifo->fifo_size)
  {
    writec -= fifo->fifo_size;
    wr_mask = wr_mask ^ WRAP_SET_MASK;
  }

  writec |= wr_mask;

  *(fifo->writec) = writec;
}

static inline int pull_remote_writec(FCB* fifo)
{
  if (fifo->remote_writec == NULL)
  {
    return 0;
  }

  if (fifo->use_dma)
  {
    hw_dma_receive(fifo->writec, fifo->remote_writec, 1, fifo->dma, BLOCKING);
  } else
  {
    memcpy_int(fifo->writec, fifo->remote_writec, 1);
  }
  return 1;
}

static inline int push_local_writec(FCB* fifo)
{
  if (fifo->remote_writec == NULL)
  {
    return 0;
  }

  if (fifo->use_dma)
  {
    fifo->write_id = hw_dma_send(fifo->remote_writec, fifo->writec, 1, fifo->dma, NON_BLOCKING);
  } else
  {
    memcpy_int(fifo->remote_writec, fifo->writec, 1);
  }
  return 1;
}

static inline int calc_tokens_of_space_to_buffer_end(FCB* fifo)
{
  return fifo->fifo_size - (*(fifo->writec) & WR_RD_MASK);
}

static inline int calc_tokens_of_data_to_buffer_end(FCB* fifo)
{
  return fifo->fifo_size - (*(fifo->readc) & WR_RD_MASK);
}

static inline int pull_remote_buffer(FCB* fifo)
{
  int data;

  if (fifo->remote_buffer == NULL)
  {
    return 0;
  }
  data = calc_tokens_of_data_to_buffer_end(fifo);

  if (data < fifo->cons_token_rate)
  {
    if (fifo->use_dma)
    {
      hw_dma_receive(fifo->buffer, calc_remote_buffer_read_pointer(fifo),
                     (data * fifo->token_size) >> 2, fifo->dma, NON_BLOCKING);
      hw_dma_receive((void*)((void*)fifo->buffer + (data * fifo->token_size)),
                     fifo->remote_buffer,
                     ((fifo->cons_token_rate - data) * fifo->token_size) >> 2,
                     fifo->dma, BLOCKING);
    } else
    {
      memcpy_int(fifo->buffer, calc_remote_buffer_read_pointer(fifo),
                 (data * fifo->token_size) >> 2);
      memcpy_int((void*)((void*)fifo->buffer + (data * fifo->token_size)),
                 fifo->remote_buffer,
                 ((fifo->cons_token_rate - data) * fifo->token_size) >> 2);
    }
  } else
  {
    if (fifo->use_dma)
    {
      hw_dma_receive(fifo->buffer, calc_remote_buffer_read_pointer(fifo),
                     (fifo->cons_token_rate * fifo->token_size) >> 2,
                     fifo->dma, BLOCKING);
    } else
    {
      memcpy_int(fifo->buffer, calc_remote_buffer_read_pointer(fifo),
                 (fifo->cons_token_rate * fifo->token_size) >> 2);
    }
  }
  return 1;
}

static inline int push_local_buffer(FCB* fifo)
{
  int   space;
  void* remote_buffer_wp;

  if (fifo->remote_buffer == NULL)
  {
    return 0;
  }
  space = calc_tokens_of_space_to_buffer_end(fifo);
  remote_buffer_wp = calc_remote_buffer_write_pointer(fifo);

  if (space < fifo->prod_token_rate)
  {
    if (fifo->use_dma)
    {
      hw_dma_send(remote_buffer_wp, fifo->buffer,
                  (space * fifo->token_size) >> 2, fifo->dma, NON_BLOCKING);
      fifo->write_id = hw_dma_send(fifo->remote_buffer,
                  (void*)((void*)fifo->buffer + (space * fifo->token_size)),
                  ((fifo->prod_token_rate - space) * fifo->token_size) >> 2,
                  fifo->dma, NON_BLOCKING);
    } else
    {
      memcpy_int(remote_buffer_wp, fifo->buffer,
                 (space * fifo->token_size) >> 2);
      memcpy_int(fifo->remote_buffer,
                 (void*)((void*)fifo->buffer + (space * fifo->token_size)),
                 ((fifo->prod_token_rate - space) * fifo->token_size) >> 2);
    }
  } else
  {
    if (fifo->use_dma)
    {
      fifo->write_id = hw_dma_send(remote_buffer_wp, fifo->buffer,
                  (fifo->prod_token_rate * fifo->token_size) >> 2,
                  fifo->dma, NON_BLOCKING);
    } else
    {
      memcpy_int(remote_buffer_wp, fifo->buffer,
                 (fifo->prod_token_rate * fifo->token_size) >> 2);
    }
  }

  if (!fifo->same_memory)
  {
    // make sure writes are complete before the admin can be updated
    if (fifo->use_dma)
    {
      // receive can be NON_BLOCKING as it uses the same DMA and NoC connection as the
      // send from the subsequent data release, ensuring that order is maintained.
      hw_dma_receive(fifo->buffer, remote_buffer_wp, 1, fifo->dma, NON_BLOCKING);
    } else
    {
      memcpy_int(fifo->buffer, remote_buffer_wp, 1);
    }
  }

  return 1;
}

static inline void* claim_write_pointer(FCB* fifo)
{
  void* index = calc_buffer_write_pointer(fifo);

  fifo->claimed_space++;
  return index;
}

static inline void* claim_read_pointer(FCB* fifo)
{
  void* index = calc_buffer_read_pointer(fifo);

  fifo->claimed_data++;
  return index;
}

/*
 * Export Functions
 */
void fifo_remove(FCB* fcb)
{
  // Free all the allocated memory.
  if ((fcb->allocated_memory & BUFFER_MEMORY) == BUFFER_MEMORY)
  {
    // avoid compiler warnings.
    mk_free((void *)fcb->buffer);
  }
  if ((fcb->allocated_memory & WRITEC_MEMORY) == WRITEC_MEMORY)
  {
    // avoid compiler warnings.
    mk_free((void*)fcb->writec);
  }
  if ((fcb->allocated_memory & READC_MEMORY) == READC_MEMORY)
  {
    // avoid compiler warnings.
    mk_free((void*)fcb->readc);
  }
  mk_free(fcb);
}

FCB* fifo_add(
  int* writec, int* readc, int* remote_writec, int* remote_readc,
  void* buffer, void* remote_buffer,
  int fifo_size, int token_size,
  int max_prod_token_rate, int max_cons_token_rate,
  int dma_id, int same_memory, int self_fifo, int init)
{
  FCB* fcb = (FCB*)mk_malloc(sizeof(FCB));

  fcb->allocated_memory = NO_ALLOC;

  if (buffer == NULL)
  {
    buffer = (void*)mk_malloc(token_size * fifo_size);
    fcb->allocated_memory |= BUFFER_MEMORY;
  }
  if (writec == NULL)
  {
    writec = (int*)mk_malloc(sizeof(int));
    fcb->allocated_memory |= WRITEC_MEMORY;
  }
  if (readc == NULL)
  {
    readc = (int*)mk_malloc(sizeof(int));
    fcb->allocated_memory |= READC_MEMORY;
  }

  // Self fifo.
  fcb->self_fifo = self_fifo;

  fcb->buffer    = buffer;
  fcb->writec    = writec;
  *writec        = WRAP_RST_MASK;
  fcb->readc     = readc;
  *readc         = WRAP_RST_MASK;
  fcb->max_cons_token_rate = max_cons_token_rate;
  fcb->max_prod_token_rate = max_prod_token_rate;
  fcb->remote_buffer       = remote_buffer;
  fcb->remote_writec       = remote_writec;
  fcb->remote_readc        = remote_readc;
  fcb->fifo_size           = fifo_size;
  fcb->token_size          = token_size;
  fcb->prod_token_rate     = 0;
  fcb->cons_token_rate     = 0;
  fcb->prod_ready          = fifo_size;
  fcb->cons_ready          = 0;
  fcb->claimed_space       = 0;
  fcb->claimed_data        = 0;
  fcb->same_memory         = same_memory;
  fcb->write_id            = 0;

  if (dma_id > 0)
  {
    fcb->dma     = hw_get_dma(dma_id);
    fcb->use_dma = dma_id;
  } else
  {
    fcb->use_dma = 0;
  }

  // initializes the admin
  if (init != 0)
  {
    push_local_writec(fcb);
    push_local_readc(fcb);
  }
  return fcb;
}

int fifo_check_space(FCB* fifo)
{
  if (fifo->fifo_size == 1)
  {
    // self edge fifo
    if (fifo->self_fifo)
    {
      return 1;
    }
  }
  // If we use dma, check if the scratchpad memory is also free. 
  // Otherwise claim_space will fail and return null pointer.
  if(fifo->use_dma && !hw_dma_check_send_complete(fifo->dma, fifo->write_id)){
	  return 0;
  }
  int space = 1; // if rate is 0 return 1
  if (fifo->prod_ready < fifo->prod_token_rate)
  {
    space = calc_space(fifo);

    if (space < fifo->prod_token_rate)
    {
      if (!pull_remote_readc(fifo))
      {
        return 0;
      }
      space = calc_space(fifo);
      if (space < fifo->prod_token_rate)
      {
        return 0;
      }
    }
    fifo->prod_ready = space;
    fifo->cons_ready = fifo->fifo_size - space;
  }
  return space;
}

int fifo_check_data(FCB* fifo)
{
  if (fifo->fifo_size == 1)
  {
    // self edge fifo
    if (fifo->self_fifo)
    {
      return 1;
    }
  }
  int data = 1; // if rate is 0 return 1
  if (fifo->cons_ready < fifo->cons_token_rate)
  {
    data = calc_data(fifo);

    if (data < fifo->cons_token_rate)
    {
      if (!pull_remote_writec(fifo))
      {
        return 0;
      }
      data = calc_data(fifo);
      if (data < fifo->cons_token_rate)
      {
        return 0;
      }
    }
    fifo->cons_ready = data;
    fifo->prod_ready = fifo->fifo_size - data;
  }
  return data;
}

void fifo_pull(FCB* fifo)
{
  pull_remote_buffer(fifo);
}

void fifo_push(FCB* fifo)
{
  push_local_buffer(fifo);
}

/*
 * C-HEAP Primitives
 */
void* fifo_claim_space(FCB* fifo)
{
  if (fifo->fifo_size == 1)
  {
    // self edge fifo
    if (fifo->self_fifo)
    {
      // We can drop the volatile here, we know it is not going to be changed remotely.
      return (void *)fifo->buffer;
    }
  }
  if (fifo->prod_token_rate <= fifo->claimed_space)
  {
    return NULL;
  }
  if(fifo->use_dma){
    if(!hw_dma_check_send_complete(fifo->dma, fifo->write_id)){
      // One scratchpad position before DMA
      return NULL;
    }
  }
  return claim_write_pointer(fifo);
}

void* fifo_claim_data(FCB* fifo)
{
  if (fifo->fifo_size == 1)
  {
    // self edge fifo
    if (fifo->self_fifo)
    {
      // We can drop the volatile here, we know it is not going to be changed remotely.
      return (void *)fifo->buffer;
    }
  }
  if (fifo->cons_token_rate <= fifo->claimed_data)
  {
    return NULL;
  }
  return claim_read_pointer(fifo);
}

int fifo_release_space(FCB* fifo)
{
  if (fifo->fifo_size == 1)
  {
    // self edge fifo
    if (fifo->self_fifo)
    {
      return 1;
    }
  }
  if (fifo->claimed_data < fifo->cons_token_rate)
  {
    return 0;
  }
  fifo->cons_ready -= fifo->cons_token_rate;

  increment_local_readc(fifo);
  push_local_readc(fifo);

  return 1;
}

int fifo_release_data(FCB* fifo)
{
  if (fifo->fifo_size == 1)
  {
    // self edge fifo
    if (fifo->self_fifo)
    {
      return 1;
    }
  }
  if (fifo->claimed_space < fifo->prod_token_rate)
  {
    return 0;
  }
  fifo->prod_ready -= fifo->prod_token_rate;

  increment_local_writec(fifo);
  push_local_writec(fifo);

  return 1;
}

// Function for inserting initial tokens.
int fifo_release_data_no_push(FCB* fifo)
{
  if (fifo->fifo_size == 1)
  {
    // self edge fifo
    if (fifo->self_fifo)
    {
      return 1;
    }
  }
  if (fifo->claimed_space < fifo->prod_token_rate)
  {
    return 0;
  }
  fifo->prod_ready -= fifo->prod_token_rate;

  increment_local_writec(fifo);

  return 1;
}

void fifo_set_consumption_token_rate(FCB* fcb, int rate)
{
  // Check if the rate is valid.
  if ((rate < 0) || (rate > fcb->max_cons_token_rate))
  {
    // TODO: Print error message
    abort();
  }
  fcb->cons_token_rate = rate;
}

void fifo_set_production_token_rate(FCB* fcb, int rate)
{
  if ((rate < 0) || (rate > fcb->max_prod_token_rate))
  {
    // TODO: Print error message
    abort();
  }
  fcb->prod_token_rate = rate;
}

int fifo_get_production_token_rate(const FCB* const fcb)
{
  return fcb->prod_token_rate;
}

int fifo_get_consumption_token_rate(const FCB* const fcb)
{
  return fcb->cons_token_rate;
}

int fifo_get_max_consumption_rate(const FCB* fcb)
{
  return fcb->max_cons_token_rate;
}

int fifo_get_max_production_rate(const FCB* fcb)
{
  return fcb->max_prod_token_rate;
}

int fifo_get_token_size(const FCB* fcb)
{
  return fcb->token_size;
}
