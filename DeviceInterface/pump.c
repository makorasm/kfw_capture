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
#include "sys/sem.h"
#include "getopt.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "DeviceInterface.h"
#include "internal.h"
#include "list.h"
#include "sync_sem.h"

internal_params prm;
struct s_ProcessParam* param;
int file_pump_deinit(){

	munmap(prm.fpump.map_addr, prm.fpump.file_size);
	sem_destroy(&prm.fpump.stop_sem);
	memset(&prm,0, sizeof(prm));
	return 0;

}
int file_pump_init(){

	int fl_id;
	struct stat st;

	if(param->VideoParam.p_params.file_pump_params.pump_read_callback==NULL){
		
		errno=EINVAL;

		perror("file_pump_init 0");
		return -1;

	}


	fl_id=open(param->VideoParam.p_params.file_pump_params.sf_name, O_RDONLY);
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
	if(param->VideoParam.p_params.file_pump_params.buf_size > st.st_size){
		prm.fpump.buf_size=st.st_size;
	}else{

		prm.fpump.buf_size=param->VideoParam.p_params.file_pump_params.buf_size;
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
	prm.fpump.pump_read_callback=param->VideoParam.p_params.file_pump_params.pump_read_callback;
	prm.fpump.pump_read_callback_data=param->VideoParam.p_params.file_pump_params.pump_read_callback_data;
	param->VideoParam.p_params.file_pump_params.stop_sem=&prm.fpump.stop_sem;
	close(fl_id);
	return 0;
}

int net_pump_init(){
	

	return -1;
}
//**********************************************************************
//**********************************************************************
int omx_pump_init(){
	char fifo_name[256];
	char fifo_name1[256];
	prm.omxpump.shm_id=shmget(1408, 1024*1024*2, IPC_CREAT|00777);
	if(prm.omxpump.shm_id==-1){
		perror("shmget");
		return -1;
	}
	prm.omxpump.shm_point=shmat(prm.omxpump.shm_id, NULL, 0);
	if(prm.omxpump.shm_point==(void*)-1){
		perror("shmat");
		shmctl(prm.omxpump.shm_id, IPC_RMID, NULL);
		return -1;
	}
	
	getcwd(fifo_name, 256);
	strcat(fifo_name, "/sync.fifo");
	if(mkfifo(fifo_name, 00777)==-1){
		if(errno!=EEXIST){
			perror("mkfifo");
			shmdt(prm.omxpump.shm_point);
			shmctl(prm.omxpump.shm_id, IPC_RMID, NULL);
			return -1;
		}
	}
	getcwd(fifo_name1, 256);
	strcat(fifo_name1, "/cmd.fifo");
	if(mkfifo(fifo_name1, 00777)==-1){
		if(errno!=EEXIST){
			perror("mkfifo 1");
			shmdt(prm.omxpump.shm_point);
			shmctl(prm.omxpump.shm_id, IPC_RMID, NULL);
			unlink(fifo_name);
			return -1;
		}
	}
//omxpump struct initialization
	if(sem_init(&prm.omxpump.stop_sem, 0, 0)==-1){

			perror("sem_init");
			shmdt(prm.omxpump.shm_point);
			shmctl(prm.omxpump.shm_id, IPC_RMID, NULL);
			unlink(fifo_name);
			unlink(fifo_name1);
			return -1;
	}
	param->VideoParam.p_params.omx_pump_params.stop_sem=&prm.omxpump.stop_sem;
	//INIT_LIST_HEAD(&param.VideoParam.p_params.omx_pump_params.callback_chain.entry);
	prm.omxpump.sync_sem=semget((key_t)1409, 1, 0666|IPC_CREAT);
  if(prm.omxpump.sync_sem == -1){
		perror("seget");
		shmdt(prm.omxpump.shm_point);
		shmctl(prm.omxpump.shm_id, IPC_RMID, NULL);
		unlink(fifo_name);
		unlink(fifo_name1);
		sem_destroy(&prm.omxpump.stop_sem);
			return -1;
	}

	return 0;
}

void omx_pump_deinit(){
	
	char fifo_name[256];
	shmdt(prm.omxpump.shm_point);
	shmctl(prm.omxpump.shm_id, IPC_RMID, NULL);
	sem_destroy(&prm.omxpump.stop_sem);
	getcwd(fifo_name, 256);
	strcat(fifo_name, "/sync.fifo");
	unlink(fifo_name);
	getcwd(fifo_name, 256);
	strcat(fifo_name, "/cmd.fifo");
	unlink(fifo_name);
	semctl(prm.omxpump.sync_sem, 0, IPC_RMID, NULL);	

}

void* omx_source_thread(void* prms){
	
	pinternal_params prm=(pinternal_params)prms;
	pinternal_omxpump iomx_pump=&prm->omxpump;
	pomxpump eomx_pump=&param->VideoParam.p_params.omx_pump_params;
	struct list_head* temp_list=eomx_pump->callback_chain.next;
	pipe_cmd p_cmd;
	pcall_chain call_ent;
	while(!iomx_pump->stop_cond){
	//	fsync(iomx_pump->synk_pipe_id);
		if(read(iomx_pump->synk_pipe_id, &p_cmd, sizeof(p_cmd))<sizeof(p_cmd)){
			perror("synk pipe read");
			sem_post(eomx_pump->stop_sem);
			pthread_exit(NULL);
			return NULL;
		}
		free_sem(iomx_pump->sync_sem);
		temp_list=eomx_pump->callback_chain.next;
		printf("PUMPLIB: read data %d %d\n", p_cmd.bf_size, p_cmd.bf_offset);
		while(&eomx_pump->callback_chain != temp_list){
			printf("PUMPLIB: list %p head %p next %p\n", 
					temp_list, &eomx_pump->callback_chain, eomx_pump->callback_chain.next);	
			call_ent=list_entry(temp_list,struct _call_chain,entry);
			call_ent->read_callback(iomx_pump->shm_point+p_cmd.bf_offset, p_cmd.bf_size, call_ent->callback_data);
			temp_list=temp_list->next;

		}
			
	}
	sem_post(eomx_pump->stop_sem);
	return NULL;
}


int omx_pump_start(){
	char fifo_name[256];
	pthread_attr_t attr;
	pinternal_omxpump iomx_pump=&prm.omxpump;
	pomxpump eomx_pump=&param->VideoParam.p_params.omx_pump_params;
	pid_t pid;
	char proc_path[256];	
	
printf("INIT LIST: %p %p\n", 	&eomx_pump->callback_chain, eomx_pump->callback_chain.next);
	if(list_empty(&param->VideoParam.p_params.omx_pump_params.callback_chain)){
		fprintf(stderr, "ERROR: Callback chain list is epty!");
		return -1;
  }
 //Here will be fork() 
 //
 //
	pid=fork();
	switch(pid){
	
		case 0:
			getcwd(proc_path, 256);
			strcat(proc_path, "/kfw_cap_a8host_debug.xv5T");
			if(execl(proc_path, "kfw_cap_a8host_debug.xv5T", NULL)==-1){
				perror("start omx_test");
				return -1;
			}
			break;
		case -1:
			perror("fork");
			return -1;
			break;
		default:
			eomx_pump->omx_pid=pid;
			break;

	
	}
	getcwd(fifo_name, 256);
	strcat(fifo_name, "/sync.fifo");
 	prm.omxpump.synk_pipe_id=open(fifo_name, O_RDONLY|O_SYNC);
	if(prm.omxpump.synk_pipe_id==-1){
		perror("open synk fifo");
		return -1;
	}
 
	getcwd(fifo_name, 256);
	strcat(fifo_name, "/cmd.fifo");
 	prm.omxpump.cmd_pipe_id=open(fifo_name, O_WRONLY);
	if(prm.omxpump.synk_pipe_id==-1){
		perror("open cmd fifo");
		return -1;
	}
	iomx_pump->stop_cond=0;
	pthread_attr_init(&attr);
	if(pthread_create(&prm.omxpump.thr_id, &attr,omx_source_thread, &prm )==-1){

		perror("thread ctreate");
		close(prm.omxpump.cmd_pipe_id);
		close(prm.omxpump.synk_pipe_id);
		return -1;
	}
	return 0;
}
//**********************************************************************
//**********************************************************************
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
	switch(param->VideoParam.p_type){
		
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
			if(omx_pump_init()==-1){
				fprintf(stderr, "omx_pump_init FAULT!\n");
				err=-1;
				break;
			}
			if(omx_pump_start()==-1){
				fprintf(stderr, "omx_pump_start FAULT!!\n");
				omx_pump_deinit();
				err=-1;
				break;
			}
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

	switch(param->VideoParam.p_type){
		
		case eFILE_PUMP:
			prm.fpump.stop_cond=1;
			//sem_wait(&prm.fpump.stop_sem);
			pthread_join(prm.fpump.thr_id,NULL);
			break;
		case eNET_PUMP:
			break;
		case eOMX_PUMP:
			prm.omxpump.stop_cond=1;
			pthread_join(prm.omxpump.thr_id,NULL);
			kill(param->VideoParam.p_params.omx_pump_params.omx_pid, 
					SIGKILL);
			omx_pump_deinit();
			break;
		case eDEV_PUMP:
			break;
		default:
			break;
	
	
	}
}
// at the begining calling GetProcessParam and filling struct with process param, later we only change param and apply them
int SetProcessParam(struct s_ProcessParam *ProcessParam){
	
	if(ProcessParam)param=ProcessParam;// memcpy(&param, ProcessParam, sizeof(param));
	return 0;
}
void GetProcessParam(struct s_ProcessParam * ProcessParam){
	
	//if(ProcessParam) memcpy(ProcessParam, &param, sizeof(param));

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

