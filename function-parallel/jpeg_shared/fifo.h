#ifndef __FIFO_H__
#define __FIFO_H__
/**
 * \defgroup libfifo CompSOC C-Heap based FIFO implementation.
 * @{
 *
 * FIFO
 *
 * GENERAL SCHEME WRITE TO FIFO
 *  - fifo_check_space()
 *  - fifo_claim_space()
 *  - fifo_push()
 *  - fifo_release_data()
 *
 * GENERAL SCHEME READ FROM FIFO FOR A CSDF TASK
 *  - fifo_check_data()
 *  - fifo_claim_data()
 *  - fifo_pull()
 *  - fifo_release_space()
 *
 *
 * Copyright 2014 Eindhoven University of Technology
 */

/**
 * Anonymous forward declaration of the FIFO
 * Control block.
 */
typedef struct FCB_s FCB;

/**
 * @param fcb The Control Block of the FIFO.
 *
 * Free the FIFO administration.
 */
void fifo_remove(FCB* fcb) __attribute__((nonnull));

/**
 * @param writec              Local Write counter.
 * @param readc               Local Read counter.
 * @param remote_writec       Remote Write counter.
 * @param remote_readc        Remote Read counter.
 * @param buffer              Local FIFO buffer (or scratchpad).
 * @param remote_buffer       Remote FIFO buffer.
 * @param fifo_size           Size of the FIFO in tokens.
 * @param token_size          Size of a Token (should be multiple of 4 bytes).
 * @param max_cons_token_rate The maximum number of tokens consumed per fire.
 * @param max_prod_token_rate The maximum number of tokens produced per fire.
 * @param dma_id              The ID of the dma to use (0 using mmio)
 * @param same_memory         Remote buffer and remote writec are in same memory.
 * @param self_fifo           FIFO is a self FIFO (memory optimization).
 * @param init                Initialize the remote counter(s).
 *
 * Create a FIFO.
 *
 * @returns The Control Block of the newly created FIFO.
 */
FCB* fifo_add(int*  writec,
              int*  readc,
              int*  remote_writec,
              int*  remote_readc,
              void* buffer,
              void* remote_buffer,
              int   fifo_size,
              int   token_size,
              int   max_prod_token_rate,
              int   max_cons_token_rate,
              int   dma_id,
              int   same_memory,
              int   self_fifo,
              int   init);

/**
 * @param fcb The Control Block of the FIFO.
 * @param rate The new consumption rate.
 *
 * Set the consumption rate off the FIFO
 */
void fifo_set_consumption_token_rate(FCB* fcb,
                                     int  rate) __attribute__((nonnull(1)));

/**
 * @param fcb  The Control Block of the FIFO.
 * @param rate The new production rate.
 *
 * Set the production rate off the FIFO
 */
void fifo_set_production_token_rate(FCB* fcb,
                                    int  rate) __attribute__((nonnull(1)));

/**
 * @param fcb  The Control Block of the FIFO
 *
 * @returns the current consumption rate.
 */
int fifo_get_consumption_token_rate(const FCB* const fcb) __attribute__((nonnull(1)));

/**
 * @param fcb  The Control Block of the FIFO
 *
 * @returns the current production rate.
 */
int fifo_get_production_token_rate(const FCB* const fcb) __attribute__((nonnull(1)));

/**
 * @param fifo The Control Block of the FIFO
 *
 * checks if there is enough space for an iteration of the producing task
 * returns 0 if not enough space
 * returns 1 if enough space or fifo_prod_rate==0
 */
int fifo_check_space(FCB* fifo) __attribute__((nonnull,used));

/**
 * @param fifo The Control Block of the FIFO
 *
 * checks if there is enough data for an iteration of the consuming task
 * returns 0 if not enough data
 * returns 1 if enough data or fifo_cons_rate==0
 */
int fifo_check_data(FCB* fifo) __attribute__((nonnull,used));

/**
 * @param fifo  The Control Block of the FIFO
 *
 * claims 1 token of space for the task to produce data into
 * assumes that a check has been made using fifo_check_fifo_space()
 * returns NULL if number of claimed tokens exceeds fifo_prod_rate
 * returns pointer to 1 token of local space
 * call again to return the next pointer
 */
void* fifo_claim_space(FCB* fifo) __attribute__((nonnull,used));

/**
 * @param fifo The Control Block of the FIFO
 *
 * claims 1 token of data for the task to consume data from
 * assumes that a check has been made using fifo_check_fifo_data()
 * returns NULL if number of claimed tokens exceeds fifo_cons_rate
 * returns pointer to 1 token of local data
 * call again to return the next pointer
 */
void* fifo_claim_data(FCB* fifo) __attribute__((nonnull,used));

/**
 * @param fifo The Control Block of the FIFO
 *
 * releases the token space from 1 iteration of task consumption
 * returns 0 if all the data for 1 iteration has not been claimed
 * returns 1 if space is successfully released
 */
int fifo_release_space(FCB* fifo) __attribute__((nonnull,used));

/**
 * @param fifo The Control Block of the FIFO
 *
 * releases the token data from 1 iteration of task production
 * returns 0 if all the space for 1 iteration has not been claimed
 * returns 1 if data is successfully released
 */
int fifo_release_data(FCB* fifo) __attribute__((nonnull,used));

/**
 * @param fifo The Control Block of the FIFO
 *
 * reads remote fifo into local buffer if necessary
 */
void fifo_pull(FCB* fifo) __attribute__((nonnull,used));

/**
 * @param fifo The Control Block of the FIFO
 *
 * writes remote fifo into remote buffer if necessary
 */
void fifo_push(FCB* fifo) __attribute__((nonnull,used));

/**
 * @param fcb  The Control Block of the FIFO
 *
 * @returns the maximum consumption rate.
 */
int fifo_get_max_consumption_rate(const FCB* fcb) __attribute__((nonnull,used));

/**
 * @param fcb  The Control Block of the FIFO
 *
 * @returns the maximum production rate.
 */
int fifo_get_max_production_rate(const FCB* fcb) __attribute__((nonnull,used));

int fifo_get_token_size(const FCB* fcb) __attribute__((nonnull,used));

/** @} */
#endif // ifndef __FIFO_H__
