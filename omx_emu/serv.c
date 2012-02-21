/*
 * =====================================================================================
 *
 *       Filename:  serv.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/20/2012 11:34:07 AM
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
#include "../DeviceInterface/DeviceInterface.h"
#include "../DeviceInterface/internal.h"
#include "serv.h"

int init_comm_resource(pcomm_res res){

	char fifo_name[256];
	pthread_attr_t attr;


	getcwd(fifo_name, 256);
	strcat(fifo_name, "/sync.fifo");
	res->sync_pipe_id=open(fifo_name, O_WRONLY);
	if(res->sync_pipe_id == -1){

		perror("omx open sync pipe");
		return -1;
	}

	printf("OMX init: sync pipe is opened %d\n", res->sync_pipe_id);

	getcwd(fifo_name, 256);
	strcat(fifo_name, "/cmd.fifo");
	res->cmd_pipe_id=open(fifo_name, O_RDONLY);
	if(res->cmd_pipe_id == -1){

		perror("omx open cmd pipe");
		close(res->sync_pipe_id);
		return -1;
	}

	printf("OMX init: cmd  pipe is opened %d\n", res->cmd_pipe_id);

	res->shm_id = shmget(1408, 1024*1024*2, 00777);

	if(res->shm_id == -1){

		perror("omx shmget");
		close(res->sync_pipe_id);
		close(res->cmd_pipe_id);
		return -1;

	}
	
	printf("OMX init: shared mem opened %d\n", res->shm_id);

	res->shm_point = shmat(res->shm_id, NULL, 0);

	if(res->shm_point == (void*)-1){

		perror("omx shmat");
		shmctl(res->shm_id, IPC_RMID, NULL);
		close(res->sync_pipe_id);
		close(res->cmd_pipe_id);
		return -1;

	}
	
	printf("OMX init: shared mem mapped %p\n", res->shm_point);

	pthread_attr_init(&attr);
	if(pthread_create(&res->cmd_thread_id, &attr, cmd_thread, res )!=0){

		perror("omx cmd thread create");
		shmdt(res->shm_point);
		shmctl(res->shm_id, IPC_RMID, NULL);
		close(res->sync_pipe_id);
		close(res->cmd_pipe_id);
		return -1;
	}


	printf("OMX init: command thread created %d\n", res->cmd_thread_id);

	return 0;
}



int write_comm_data(char* d_point,  ppipe_cmd p_cmd, pcomm_res res){
	int ret_val=0;
	memcpy(res->shm_point+p_cmd->bf_offset, d_point, p_cmd->bf_size);

	if((ret_val=write(res->sync_pipe_id, p_cmd,sizeof(pipe_cmd) ))<0){
		
		perror("write sync fifo");
		return -1;
	}else if(ret_val == 0){
	
		fprintf(stderr, "sync fifo closed!!!\n");
		return -2;
	}

	printf("OMX write data: addr %p size %d\n", res->shm_point+p_cmd->bf_offset, 
			p_cmd->bf_size);
	return 0;
}

void* cmd_thread(void* param){

	printf("OMX command thread enter...\n");

	return NULL;
}
