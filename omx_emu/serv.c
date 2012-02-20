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
#include "../DeviceInterface/internal.h"
#include "serv.h"

int init_comm_resource(pcomm_res res){

	char fifo_name[256];
	pthread_attr_t attr;


	getcwd(fifo_name);
	strcat(fifo_name, "/sync.fifo");
	res->sync_pipe_id=open(fifo_name, O_WRONLY);
	if(res->sync_pipe_id == -1){

		perror("omx open sync pipe");
		return -1;
	}

	getcwd(fifo_name);
	strcat(fifo_name, "/cmd.fifo");
	res->cmd_pipe_id=open(fifo_name, O_RDONLY);
	if(res->cmd_pipe_id == -1){

		perror("omx open cmd pipe");
		close(res->sync_pipe_id);
		return -1;
	}

	res->shm_id = shmget(1408, 1024*1024*2, 00777);

	if(res->shm_id == -1){

		perror("omx shmget");
		close(res->sync_pipe_id);
		close(res->cmd_pipe_id);
		return -1;

	}
	
	res->shm_point = shmat(res->shm_id, NULL, 0);

	if(res->shm_point == (void*)-1){

		perror("omx shmat");
		shmctl(res->shm_id, IPC_RMID, NULL);
		close(res->sync_pipe_id);
		close(res->cmd_pipe_id);
		return -1;

	}
	
	pthread_attr_init(&attr);
	if(pthread_create(&res->cmd_thread_id, &attr, cmd_thread, res )!=0){

		perror("omx cmd thread create");
		shmdt(res->shm_point);
		shmctl(res->shm_id, IPC_RMID, NULL);
		close(res->sync_pipe_id);
		close(res->cmd_pipe_id);
		return -1;
	}
	return 0;
}



int write_comm_data(char* d_point, unsigned bf_size, pcomm_res res){



}
