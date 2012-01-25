#ifndef _VIDEO_SHMAT_
#define _VIDEO_SHMAT_



#define SPNAME "/comm.fifo"
#define PPNAME "/param.fifo"
#define SHM_KEY 1407

typedef struct _common_res{

	int       sfile;
	void*     msfile_addr;
	unsigned  sf_offs;
	int       sync_fifo;
	int       param_fifo;
	int       shm_id;
	void*     shm_addr;
	pthread_t cmd_thread;
	pthread_t src_thread;
	sem_t     src_sem;
	char      s_fifoname[256];	
	char      p_fifoname[256];	
	unsigned  bf_size;
	unsigned  sf_size;
} common_res, *pcommon_res;

typedef enum _cmds{
cap_start, 
cap_stop, 
frame_complete, 
}cmds;



typedef struct _cmd_hdr{

cmds     cmd;
unsigned size;

}cmd_hdr, *pcmd_hdr;


#endif
