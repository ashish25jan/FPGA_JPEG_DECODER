#ifndef _JPEG_STRUCTURES_H_INCLUDED
#define _JPEG_STRUCTURES_H_INCLUDED
#include "conf.h"

typedef struct
{
    int                   vld_count;
    // Pointer to input image.
    const unsigned int    *data;
    // Pointer to output memory.
    volatile unsigned int *fb;
} JPGFile;

/* block of pixel-space values */
typedef union
{
    unsigned char block[8][8];
    unsigned char linear[64];
} PBlock;

/* block of frequency-space values */
typedef union
{
    int block[8][8];
    int linear[64];
} FBlock;

/* block of frequency-space values before IQZZ */
typedef struct
{
    unsigned long linear[64];
    PBlock        qtable;
} FValue;

/* pixel value array */
typedef struct
{
    unsigned char data[3072];
} ColorBuffer;

/* component descriptor */
typedef struct
{
    unsigned char CID, IDX;
    unsigned char HS, VS;
    unsigned char HDIV, VDIV;

    char          QT, DC_HT, AC_HT;
    int           PRED;
} cd_t;

/* settings for color conversion */
typedef struct
{
    int  MCU_sx, MCU_sy;
    int  n_comp;
    cd_t comp[3];
} SubHeader1;

/* settings for rasterization */
typedef struct
{
    int     MCU_sx, MCU_sy, MCU_column;
    int     n_comp;
    int     x_size, y_size;
    int     goodrows, goodcolumns;
    int     MCU_row;
    int     leftover;
    JPGFile fp;
} SubHeader2;

typedef struct VldHeader
{
    // input file handle
    JPGFile fp;
    // MCU size in pixels
    int     MCU_sx, MCU_sy;
    // number of components 1,3
    int     n_comp;
    //descriptors for 3 components
    cd_t    comp[3];
    // Video frame size
    int     x_size, y_size;
    // final rasterization information
    int     goodrows, goodcolumns;
    // number of MCUs left for processing
    int     leftover;
    // current position in MCU unit
    int     MCU_row, MCU_column;
    // components of above MCU blocks
    int     MCU_valid[10];
    // quantization tables
    PBlock  QTable[4];
    int     QTvalid[4];
    // down-rounded Video frame size (integer MCU)
    int     rx_size, ry_size;
    // picture size in units of MCUs
    int     mx_size, my_size;
    // frame started ? current component ?
    int     in_frame, curcomp;
    // RST check
    int     n_restarts, restart_interval, mcu_rst;
} VldHeader;
#include "5kk03.h"
#endif                                                  // _JPEG_STRUCTURES_H_INCLUDED
