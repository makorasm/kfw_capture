/*
 * =====================================================================================
 *
 *       Filename:  pump.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/12/2012 04:36:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MAX KORNYUSHIN (MAKOR), makorasm@gmail.com
 *        Company:  myself
 *
 * =====================================================================================
 */

#include "unistd.h"
#include "pthread.h"
#include "semaphore.h"
#include "sys/errno.h"
#include "sys/mman.h"
#include "sys/fcntl.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "getopt.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "DeviceInterface.h"
#include "internal.h"


internal_params prm;
struct s_ProcessParam param;
int file_pump_deinit(){

	munmap(prm.fpump.map_addr, prm.fpump.file_size);
	sem_destroy(&prm.fpump.stop_sem);
	memset(&prm,0, sizeof(prm));
	return 0;

}
int file_pump_init(){

	int fl_id;
	struct stat st;

	if(param.VideoParam.p_params.file_pump_params.pump_read_callback==NULL){
		
		errno=EINVAL;

		perror("file_pump_init 0");
		return -1;

	}


	fl_id=open(param.VideoParam.p_params.file_pump_params.sf_name, O_RDONLY);
	if(fl_id == -1){
		perror("file_pump_init 1");
		return -1;
	}
	
	if(fstat(fl_id, &st)==-1){
		perror("file_pump_init 2");
		close(fl_id);
		return -1;
	}
	prm.fpump.file_size=st.st_size;
	if(param.VideoParam.p_params.file_pump_params.buf_size > st.st_size){
		prm.fpump.buf_size=st.st_size;
	}else{

		prm.fpump.buf_size=param.VideoParam.p_params.file_pump_params.buf_size;
	}
 
	prm.fpump.map_addr=mmap(NULL,st.st_size, PROT_READ, MAP_PRIVATE, fl_id, 0 );
	if(prm.fpump.map_addr==(void*)-1){
			
		perror("file_pump_init 3");
		close(fl_id);
		return -1;
	}
	if(sem_init(&prm.fpump.stop_sem, 0, 0)==-1){
	
		perror("file_pump_init 4");
		munmap(prm.fpump.map_addr,st.st_size);
		close(fl_id);
		return -1;
	
	}
	prm.fpump.stop_cond=0;
	prm.fpump.offset=0;
	prm.fpump.pump_read_callback=param.VideoParam.p_params.file_pump_params.pump_read_callback;
	prm.fpump.pump_read_callback_data=param.VideoParam.p_params.file_pump_params.pump_read_callback_data;
	param.VideoParam.p_params.file_pump_params.stop_sem=&prm.fpump.stop_sem;
	close(fl_id);
	return 0;
}

int net_pump_init(){
	
	return -1;
}

int omx_pump_init(){

	return -1;
}

int dev_pump_init(){

	return -1;
}


void* source_thread(void* prms){
	
	pinternal_params prm=(pinternal_params)prms;
	
	while(!prm->fpump.stop_cond && (prm->fpump.offset < prm->fpump.file_size)){
		
	int currdataSize=prm->fpump.buf_size;
	if(currdataSize > (prm->fpump.file_size - prm->fpump.offset))
		currdataSize=prm->fpump.file_size - prm->fpump.offset;
			
	 prm->fpump.pump_read_callback(prm->fpump.map_addr + prm->fpump.offset, currdataSize, prm->fpump.pump_read_callback_data);

	 prm->fpump.offset += currdataSize;
	 if(prm->fpump.offset >= prm->fpump.file_size) prm->fpump.offset=0;
		
	
	}
	sem_post(&prm->fpump.stop_sem);

	return NULL;
}



int StartProcess(){
  int err=0;
	pthread_attr_t attr;
	switch(param.VideoParam.p_type){
		
		case eFILE_PUMP:
			if(file_pump_init()==-1){

				perror("StartProcess 0");
				err=-1;
				break;
			}
			prm.fpump.stop_cond=0;
			pthread_attr_init(&attr);
			//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			if(pthread_create(&prm.fpump.thr_id, &attr, source_thread, &prm)!=0){
				file_pump_deinit();				
				perror("StartProcess 1");
				err=-1;
				break;
			}
			break;
		case eNET_PUMP:
			break;
		case eOMX_PUMP:
			break;
		case eDEV_PUMP:
			break;
		default:
			err=-1;
			break;
	
	
	}
	

	return err;
}
void StopProcess(){

	switch(param.VideoParam.p_type){
		
		case eFILE_PUMP:
			prm.fpump.stop_cond=1;
			//sem_wait(&prm.fpump.stop_sem);
			pthread_join(prm.fpump.thr_id,NULL);
			break;
		case eNET_PUMP:
			break;
		case eOMX_PUMP:
			break;
		case eDEV_PUMP:
			break;
		default:
			break;
	
	
	}
}
// at the begining calling GetProcessParam and filling struct with process param, later we only change param and apply them
int SetProcessParam(struct s_ProcessParam *ProcessParam){
	
	if(ProcessParam) memcpy(&param, ProcessParam, sizeof(param));
	return 0;
}
void GetProcessParam(struct s_ProcessParam * ProcessParam){
	
	if(ProcessParam) memcpy(ProcessParam, &param, sizeof(param));

}

int GetCurrentScreenshotJPEG(char * Data, int MaxDataLen){

	return -1;
} // returning current DataLen, -1 if error
int ReadVideoData(char * Data, int DataLen){

	return -1;
} // -1,... if error
int ReadMetaData(char * Data, int DataLen){

	return -1;
} // -1,... if error
int ReadAudioData(char * Data, int DataLen){

	return -1;

} // -1,... if error

