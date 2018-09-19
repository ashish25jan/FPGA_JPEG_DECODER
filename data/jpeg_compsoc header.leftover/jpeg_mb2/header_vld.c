#ifdef PC
#include <stdlib.h>
#include <stdio.h>
#endif
#include <string.h>
#include "structures.h"
#include "huffman.h"
#include "utilities.h"

static int init_MCU ( VldHeader *header );
static int load_quant_tables ( VldHeader *header, void * fp );
static void skip_segment ( void * fi );
static int load_MCU ( VldHeader *header, FValue mcu_after_vld[10] );
static void copy_subheaders ( VldHeader *header, SubHeader1 * sh1, SubHeader2 * sh2 );

void init_header_vld ( VldHeader *header, const unsigned int *data, unsigned int *fb )
{
    unsigned int aux, mark;
    int          i;
    header->fp.fb        = fb;
    header->fp.data      = data;
    header->fp.vld_count = 0;

    init_huffman ();

    header->n_restarts       = 0;
    header->restart_interval = 0;
    header->mcu_rst          = -1;

    /* First find the SOI marker: */
    aux = get_next_MK ( &( header->fp ) );
    if ( aux != SOI_MK ) {
        aborted_stream ( &( header->fp ) );
    }

#ifdef VERBOSE
    my_fprintf ( stderr, "%zd:\tINFO:\tFound the SOI marker!\n", FTELL ( &( header->fp ) ) );
#endif

    header->in_frame = 0;
    for ( i = 0; i < 4; i++ ) {
        header->QTvalid[i] = 0;
    }

    header->restart_interval = 0;

    /* Now process segments as they appear: */
    do {
        mark = get_next_MK ( &( header->fp ) );

        switch ( mark )
        {
        case SOF_MK:
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tINFO:\tFound the SOF marker!\n", FTELL ( &( header->fp ) ) );
#endif
            header->in_frame = 1;
            get_size ( &( header->fp ) );                    /* header size, don't care */

            /* load basic image parameters */
            FGETC ( &( header->fp ) );                               /* precision, 8bit, don't care */
            header->y_size = get_size ( &( header->fp ) );
            header->x_size = get_size ( &( header->fp ) );
#ifdef VERBOSE
            my_fprintf ( stderr, "\tINFO:\tImage size is %d by %d\n", header->x_size, header->y_size );
#endif

            header->n_comp = FGETC ( &( header->fp ) );
#ifdef VERBOSE
            my_fprintf ( stderr, "\tINFO:\t" );
            switch ( header->n_comp )
            {
            case 1:
                my_fprintf ( stderr, "Monochrome" );
                break;
            case 3:
                my_fprintf ( stderr, "Color" );
                break;
            default:
                my_fprintf ( stderr, "Not a" );
                break;
            }
            my_fprintf ( stderr, " JPEG image!\n" );
#endif

            for ( i = 0; i < header->n_comp; i++ ) {
                /* component specifiers */
                header->comp[i].CID = FGETC ( &( header->fp ) );
                aux                 = FGETC ( &( header->fp ) );
                header->comp[i].HS  = first_quad ( aux );
                header->comp[i].VS  = second_quad ( aux );
                header->comp[i].QT  = FGETC ( &( header->fp ) );
            }

#ifdef VERBOSE
            if ( ( header->n_comp > 1 ) ) {
                my_fprintf ( stderr, "\tINFO:\tColor format is %d:%d:%d, H=%d\n", header->comp[0].HS * header->comp[0].VS, header->comp[1].HS * header->comp[1].VS, header->comp[2].HS * header->comp[2].VS, header->comp[1].HS );
            }
#endif

            if ( init_MCU ( header ) == -1 ) {
                aborted_stream ( &( header->fp ) );
            }

            break;

        case DHT_MK:
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tINFO:\tDefining Huffman Tables\n", FTELL ( &( header->fp ) ) );
#endif
            if ( load_huff_tables ( &( header->fp ) ) == -1 ) {
                aborted_stream ( &( header->fp ) );
            }
            break;

        case DQT_MK:
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tINFO:\tDefining Quantization Tables\n", FTELL ( &( header->fp ) ) );
#endif
            if ( load_quant_tables ( header, &( header->fp ) ) == -1 ) {
                aborted_stream ( &( header->fp ) );
            }
            break;
        case COM_MK:
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tINFO:\tSkipping comments\n", FTELL ( &( header->fp ) ) );
#endif
            skip_segment ( &( header->fp ) );
            break;
        case DRI_MK:
            get_size ( &( header->fp ) );                    // skip size
            header->restart_interval = get_size ( &( header->fp ) );
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tINFO:\tDefining Restart Interval %d\n", FTELL ( &( header->fp ) ), header->restart_interval );
#endif
            break;

        case SOS_MK:
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tINFO:\tFound the SOS marker!\n", FTELL ( &( header->fp ) ) );
#endif
            get_size ( &( header->fp ) );
            aux = FGETC ( &( header->fp ) );
            if ( aux != (unsigned int) header->n_comp ) {
                my_fprintf ( stderr, "\tERROR:\tBad component interleaving!\n" );
                aborted_stream ( &( header->fp ) );
            }

            for ( i = 0; i < header->n_comp; i++ ) {
                aux = FGETC ( &( header->fp ) );
                if ( aux != header->comp[i].CID ) {
                    my_fprintf ( stderr, "\tERROR:\tBad Component Order!\n" );
                    aborted_stream ( &( header->fp ) );
                }
                aux                   = FGETC ( &( header->fp ) );
                header->comp[i].DC_HT = first_quad ( aux );
                header->comp[i].AC_HT = second_quad ( aux );
            }
            FGETC ( &( header->fp ) );
            FGETC ( &( header->fp ) );
            FGETC ( &( header->fp ) );

            header->MCU_column = 0;
            header->MCU_row    = 0;
            clear_bits ();
            reset_prediction ( header->comp );

            if ( header->restart_interval ) {
                header->n_restarts = ceil_div ( header->mx_size * header->my_size, header->restart_interval ) - 1;
                header->leftover   = header->mx_size * header->my_size - header->n_restarts * header->restart_interval;
                header->mcu_rst    = header->n_restarts * header->restart_interval;
            }
            else {
                header->leftover = header->mx_size * header->my_size;
            }

            return;
            break;

        case EOI_MK:
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tINFO:\tFound the EOI marker before any MCU is obtained!\n", FTELL ( &( header->fp ) ) );
#endif
            aborted_stream ( &( header->fp ) );
            break;

        case EOF:
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tERROR:\tRan out of input data!\n", FTELL ( &( header->fp ) ) );
#endif
            aborted_stream ( &( header->fp ) );
            break;

        default:
            if ( ( mark & MK_MSK ) == APP_MK ) {
#ifdef VERBOSE
                my_fprintf ( stderr, "%zd:\tINFO:\tSkipping application data\n", FTELL ( &( header->fp ) ) );
#endif
                skip_segment ( &( header->fp ) );
                break;
            }
            if ( RST_MK ( mark ) ) {
                reset_prediction ( header->comp );
                break;
            }
            /* if all else has failed ... */
            my_fprintf ( stderr, "%zd:\tWARNING:\tLost Sync outside scan, %d!\n", FTELL ( &( header->fp ) ), mark );
            aborted_stream ( &( header->fp ) );
            break;
        }                                                       // end switch
    } while ( 1 );
}

void header_vld ( VldHeader *header, VldHeader *hout, FValue * mcu_after_vld, SubHeader1 * subheader1, SubHeader2 * subheader2 )
{
    unsigned int aux, mark;
    int          end = 0;
	int q = header->leftover;

    if ( header->mcu_rst > 0 ) {
        load_MCU ( header, mcu_after_vld );

        header->mcu_rst--;
        if ( header->mcu_rst % header->restart_interval == 0 ) {
            aux = get_next_MK ( &( header->fp ) );
            if ( !RST_MK ( aux ) ) {
                my_fprintf ( stderr, "%zd:\tERROR:\tLost Sync after interval!\n", FTELL ( &( header->fp ) ) );
                aborted_stream ( &( header->fp ) );
            }
#ifdef VERBOSE
            my_fprintf ( stderr, "%zd:\tINFO:\tFound Restart Marker\n", FTELL ( &( header->fp ) ) );
#endif
            reset_prediction ( header->comp );
            clear_bits ();
        }
        copy_subheaders ( header, subheader1, subheader2 );
        return;
    }
	else if ( header->leftover > q/2 ) {          // still more MCUs to process
        load_MCU ( header, mcu_after_vld );
        header->leftover--;
        copy_subheaders ( header, subheader1, subheader2 );
        return;
    }
    else if ( header->leftover <= q/2 ) {          // still more MCUs to process
        load_MCU ( header, mcu_after_vld );
        header->leftover--;
        copy_subheaders ( header, subheader1, subheader2 );
        return;
    }
    else if ( header->leftover == 0 ) {         //no more MCU to process
        /* Now process segments as they appear: */
        header->in_frame = 0;                   //finished MCUs
        do {
            mark = get_next_MK ( &( header->fp ) );

            switch ( mark )
            {
            case COM_MK:
#ifdef VERBOSE
                my_fprintf ( stderr, "%zd:\tINFO:\tSkipping comments\n", FTELL ( &( header->fp ) ) );
#endif
                skip_segment ( &( header->fp ) );
                break;

            case EOI_MK:
#ifdef VERBOSE
                my_fprintf ( stderr, "%zd:\tINFO:\tFound the EOI marker!\n", FTELL ( &( header->fp ) ) );
#endif

                end              = 1;
                header->leftover = -1;
                break;

            case EOF:
#ifdef VERBOSE
                my_fprintf ( stderr, "%zd:\tERROR:\tRan out of input data!\n", FTELL ( &( header->fp ) ) );
#endif
                aborted_stream ( &( header->fp ) );

            default:
                if ( ( mark & MK_MSK ) == APP_MK ) {
#ifdef VERBOSE
                    my_fprintf ( stderr, "%zd:\tINFO:\tSkipping application data\n", FTELL ( &( header->fp ) ) );
#endif
                    skip_segment ( &( header->fp ) );
                    break;
                }
                if ( RST_MK ( mark ) ) {
                    reset_prediction ( header->comp );
                    break;
                }
                /* if all else has failed ... */
                my_fprintf ( stderr, "%zd:\tWARNING:\tLost Sync outside scan, %d!\n", FTELL ( &( header->fp ) ), mark );
                aborted_stream ( &( header->fp ) );
                break;
            }                                                   /* end switch */
        } while ( !end );
    }
    // Copy output.
    *hout = *header;
}

/*
 * initialize locations
 */
int init_MCU ( VldHeader *header )
{
    int i, j, k, n, hmax = 0, vmax = 0;

    for ( i = 0; i < 10; i++ ) {
        header->MCU_valid[i] = -1;
    }

    k = 0;

    for ( i = 0; i < header->n_comp; i++ ) {
        if ( header->comp[i].HS > hmax ) {
            hmax = header->comp[i].HS;
        }
        if ( header->comp[i].VS > vmax ) {
            vmax = header->comp[i].VS;
        }
        n = header->comp[i].HS * header->comp[i].VS;

        header->comp[i].IDX = k;
        for ( j = 0; j < n; j++ ) {
            header->MCU_valid[k] = i;
            k++;
            if ( k == 10 ) {
                my_fprintf ( stderr, "\tERROR:\tMax subsampling exceeded!\n" );
                return -1;
            }
        }
    }

    header->MCU_sx = 8 * hmax;
    header->MCU_sy = 8 * vmax;
    for ( i = 0; i < header->n_comp; i++ ) {
        header->comp[i].HDIV = ( hmax / header->comp[i].HS > 1 );       /* if 1 shift by 0 */
        header->comp[i].VDIV = ( vmax / header->comp[i].VS > 1 );       /* if 2 shift by one */
    }

    header->mx_size = ceil_div ( header->x_size, header->MCU_sx );
    header->my_size = ceil_div ( header->y_size, header->MCU_sy );
    header->rx_size = header->MCU_sx * floor_div ( header->x_size, header->MCU_sx );
    header->ry_size = header->MCU_sy * floor_div ( header->y_size, header->MCU_sy );

    return 0;
}

/*----------------------------------------------------------*/
/* loading and allocating of quantization table             */
/* table elements are in ZZ order (same as unpack output)   */
/*----------------------------------------------------------*/
int load_quant_tables ( VldHeader *header, void * fi )
{
    char         aux;
    unsigned int size, n, i, id, x;

    size = get_size ( fi );             /* this is the table's size */
    n    = ( size - 2 ) / 65;

    for ( i = 0; i < n; i++ ) {
        aux = FGETC ( fi );
        if ( first_quad ( aux ) > 0 ) {
            my_fprintf ( stderr, "\tERROR:\tBad QTable precision!\n" );
            return -1;
        }
        id = second_quad ( aux );
#ifdef VERBOSE
        my_fprintf ( stderr, "\tINFO:\tLoading table %d\n", id );
#endif
        header->QTvalid[id] = 1;
        for ( x = 0; x < 64; x++ ) {
            header->QTable[id].linear[x] = FGETC ( fi );
        }
        /*
           -- This is useful to print out the table content --
           for (x = 0; x < 64; x++)
           my_fprintf(stderr, "%d\n", QTable[id]->linear[x]);
         */
    }
    return 0;
}

void skip_segment ( void * fi )
{                                                               /* skip a segment we don't want */
    unsigned int size;
    char         tag[5];
    int          i;

    size = get_size ( fi );
    if ( size > 5 ) {
        for ( i = 0; i < 4; i++ ) {
            tag[i] = FGETC ( fi );
        }
        tag[4] = '\0';
#ifdef VERBOSE
        my_fprintf ( stderr, "\tINFO:\tTag is %s\n", tag );
#endif
        size -= 4;
    }
    FSEEK ( fi, size - 2, SEEK_CUR );
}

int load_MCU ( VldHeader *header, FValue * mcu_after_vld )
{
    if ( header->MCU_column == header->mx_size ) {
        header->MCU_column = 0;
        header->MCU_row++;
        if ( header->MCU_row == header->my_size ) {
            header->in_frame = 0;
            return 0;
        }
#ifdef VERBOSE
        my_fprintf ( stderr, "%zd:\tINFO:\tProcessing stripe %d/%d\n", FTELL ( &( header->fp ) ), header->MCU_row + 1, header->my_size );
#endif
    }

    for ( header->curcomp = 0; header->MCU_valid[header->curcomp] != -1; header->curcomp++ ) {
        unsigned int  i, run, cat;
        int           value;
        unsigned char symbol;
        FValue        *fvalue = &mcu_after_vld[header->curcomp];
        int           select  = header->MCU_valid[header->curcomp];

        /* Init the block with 0's: */
        for ( i = 0; i < 64; i++ ) {
            fvalue->linear[i] = 0;
        }

        /* First get the DC coefficient: */
        symbol = get_symbol ( &( header->fp ), HUFF_ID ( DC_CLASS, header->comp[select].DC_HT ) );
        if ( symbol == 0 ) {
            value = 0;
        }
        else{
            value = reformat ( get_bits ( &( header->fp ), symbol ), symbol );
        }

        value                    += header->comp[select].PRED;
        header->comp[select].PRED = value;
        fvalue->linear[0]         = value;

        /* Now get all 63 AC values: */
        for ( i = 1; i < 64; i++ ) {
            symbol = get_symbol ( &( header->fp ), HUFF_ID ( AC_CLASS, header->comp[select].AC_HT ) );
            if ( symbol == HUFF_EOB ) {
                break;
            }
            if ( symbol == HUFF_ZRL ) {
                i += 15;
                continue;
            }
            cat   = symbol & 0x0F;
            run   = ( symbol >> 4 ) & 0x0F;
            i    += run;
            value = reformat ( get_bits ( &( header->fp ), cat ), cat );

            fvalue->linear[i] = value;
        }
        memcpy ( &fvalue->qtable, &( header->QTable[(int) header->comp[select].QT] ), sizeof ( PBlock ) );
    }

    /* cut last row/column as needed */
    if ( ( header->y_size != header->ry_size ) && ( header->MCU_row == ( header->my_size - 1 ) ) ) {
        header->goodrows = header->y_size - header->ry_size;
    }
    else{
        header->goodrows = header->MCU_sy;
    }

    if ( ( header->x_size != header->rx_size ) && ( header->MCU_column == ( header->mx_size - 1 ) ) ) {
        header->goodcolumns = header->x_size - header->rx_size;
    }
    else{
        header->goodcolumns = header->MCU_sx;
    }

    header->MCU_column++;

    return 1;
}

void copy_subheaders ( VldHeader *header, SubHeader1 * subheader1, SubHeader2 * subheader2 )
{
    subheader1->MCU_sx = header->MCU_sx;
    subheader1->MCU_sy = header->MCU_sy;
    subheader1->n_comp = header->n_comp;
    memcpy ( &subheader1->comp, &( header->comp ), 3 * sizeof ( cd_t ) );

    subheader2->leftover    = header->leftover;
    subheader2->goodrows    = header->goodrows;
    subheader2->goodcolumns = header->goodcolumns;
    subheader2->x_size      = header->x_size;
    subheader2->y_size      = header->y_size;
    subheader2->MCU_sx      = header->MCU_sx;
    subheader2->MCU_sy      = header->MCU_sy;
    subheader2->MCU_column  = header->MCU_column;
    subheader2->MCU_row     = header->MCU_row;
    subheader2->n_comp      = header->n_comp;
    subheader2->fp          = header->fp;
}

size_t get_num_pixels ( VldHeader *header )
{
    return header->x_size * header->y_size;
}
