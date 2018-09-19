OPTFLAGS=-O3 -Wall -std=c99 -Wl,--gc-sections


# download image, or not
DO_IMAGE_OUTPUT?=1
IMAGE_OUTPUT_WIDTH?=280
IMAGE_OUTPUT_HEIGHT?=196

# code for each tile.
MB1_DIR=jpeg_mb1
MB2_DIR=jpeg_mb2
MB3_DIR=jpeg_mb3
MB4_DIR=jpeg_mb4

# shared code and header files.
MB_SHARED=jpeg_shared

# Enable DMA mode (instead of MMIO)
USE_DMA=1

ifeq (${USE_DMA},1)
$(info DMA mode enabled)
OPTFLAGS+=-DHAS_DMA=1
else
$(info DMA mode disabled)
endif
# Upload files before the run
DATA_FILES=blue.jpg

# force use of FPGA board connected to VGA framegrabber
#USE_VGA_GRABBER?=1

# force image to be downloaded in binary form (not Huffman encoded)
#FORCE_BINARY?=1

# specify timeout
#USER_TIMEOUT?=200

include /home/emb17/material/platform/platform.mk
