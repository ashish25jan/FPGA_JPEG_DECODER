# FPGA_JPEG_DECODER

The Jpeg Decoder converts a jpeg image into bitmap image using baseline processing. The entire flow of the JPEG decoder is largely serialised, however, still there are parts such as rasterisation and printing which can utilise the benefits of parallel processing on multi-core systems. 
This repository consists of different types of parallelisation schemes (data-level, instruction level and hybrid with and without DMA) for a heterogenous FPGA. 

Different parallelisation codes are divided in to separate folders. Further analysis of different parallelisation schemes have been discussed in the report with relevant data. 

Execution:

1. Run makefile (Change directory before running makefile)

