//from 3 to 1
#define MB3_FIFO_WC1  	  (int*)(mb3_cmemout0_BASEADDR)
#define MB3_FIFO_RWC1 	  (int*)(mb1_cmemin0_pt_REMOTEADDR)
#define MB3_FIFO_RC1  	  (int*)(mb3_cmemin0_BASEADDR)
#define MB3_FIFO_RRC1  	  (int*)(NULL)
#define MB3_FIFO_BUFFER1   (int*)(mb3_cmemout0_BASEADDR+sizeof(int))
#define MB3_FIFO_RBUFFER1  (int*)(mb1_cmemin0_pt_REMOTEADDR+sizeof(int))


#define MB1_FIFO_WC  	 (int*)(mb1_cmemin0_BASEADDR)
#define MB1_FIFO_RWC 	 (int*)(NULL)
#define MB1_FIFO_RC  	 (int*)(mb1_cmemout0_BASEADDR)
#define MB1_FIFO_RRC 	 (int*)(mb3_cmemin0_pt_REMOTEADDR)
#define MB1_FIFO_BUFFER  (int*)(mb1_cmemin0_BASEADDR+sizeof(int))
#define MB1_FIFO_RBUFFER (int*)(NULL)


//from 3 to 2

#define MB3_FIFO_WC2  	  (int*)(mb3_cmemout0_BASEADDR+sizeof(int)+sizeof(struct vld3_to_1))
#define MB3_FIFO_RWC2 	  (int*)(mb2_cmemin0_pt_REMOTEADDR)
#define MB3_FIFO_RC2  	  (int*)(mb3_cmemin0_BASEADDR+sizeof(int))
#define MB3_FIFO_RRC2  	  (int*)(NULL)
#define MB3_FIFO_BUFFER2   (int*)(mb3_cmemout0_BASEADDR+2*sizeof(int)+sizeof(struct vld3_to_1))
#define MB3_FIFO_RBUFFER2  (int*)(mb2_cmemin0_pt_REMOTEADDR+sizeof(int))


#define MB2_FIFO_WC  	 (int*)(mb2_cmemin0_BASEADDR)
#define MB2_FIFO_RWC 	 (int*)(NULL)
#define MB2_FIFO_RC  	 (int*)(mb2_cmemout0_BASEADDR)
#define MB2_FIFO_RRC 	 (int*)(mb3_cmemin0_pt_REMOTEADDR+sizeof(int))
#define MB2_FIFO_BUFFER  (int*)(mb2_cmemin0_BASEADDR+sizeof(int))
#define MB2_FIFO_RBUFFER (int*)(NULL)