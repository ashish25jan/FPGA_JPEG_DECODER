

//from 1 to 2
#define MB1_FIFO_WC  	  (int*)(mb1_cmemout0_BASEADDR)
#define MB1_FIFO_RWC 	  (int*)(mb2_cmemin0_pt_REMOTEADDR)
#define MB1_FIFO_RC  	  (int*)(mb1_cmemin0_BASEADDR)
#define MB1_FIFO_RRC  	  (int*)(NULL)
#define MB1_FIFO_BUFFER   (int*)(mb1_cmemout0_BASEADDR+sizeof(int))
#define MB1_FIFO_RBUFFER  (int*)(mb2_cmemin0_pt_REMOTEADDR+sizeof(int))


#define MB2_FIFO_WC_R  	 (int*)(mb2_cmemin0_BASEADDR)
#define MB2_FIFO_RWC_R 	 (int*)(NULL)
#define MB2_FIFO_RC_R  	 (int*)(mb2_cmemout0_BASEADDR)
#define MB2_FIFO_RRC_R 	 (int*)(mb1_cmemin0_pt_REMOTEADDR)
#define MB2_FIFO_BUFFER_R  (int*)(mb2_cmemin0_BASEADDR+sizeof(int))
#define MB2_FIFO_RBUFFER_R (int*)(NULL)

//from 2 to 3
#define MB2_FIFO_WC_S  	  (int*)(mb2_cmemout0_BASEADDR+sizeof(int))
#define MB2_FIFO_RWC_S 	  (int*)(mb3_cmemin0_pt_REMOTEADDR)
#define MB2_FIFO_RC_S  	  (int*)(mb2_cmemin0_BASEADDR+sizeof(int)+2*sizeof(struct from1_to_2))
#define MB2_FIFO_RRC_S  	  (int*)(NULL)
#define MB2_FIFO_BUFFER_S   (int*)(mb2_cmemout0_BASEADDR+2*sizeof(int))
#define MB2_FIFO_RBUFFER_S  (int*)(mb3_cmemin0_pt_REMOTEADDR+sizeof(int))


#define MB3_FIFO_WC  	 (int*)(mb3_cmemin0_BASEADDR)
#define MB3_FIFO_RWC 	 (int*)(NULL)
#define MB3_FIFO_RC  	 (int*)(mb3_cmemout0_BASEADDR)
#define MB3_FIFO_RRC 	 (int*)(mb2_cmemin0_pt_REMOTEADDR+sizeof(int)+2*sizeof(struct from1_to_2))
#define MB3_FIFO_BUFFER  (int*)(mb3_cmemin0_BASEADDR+sizeof(int))
#define MB3_FIFO_RBUFFER (int*)(NULL)