/*
 * =====================================================================================
 *
 *       Filename:  omx_shm.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/23/2012 12:17:33 PM
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
#include "videoshmat.h"

int shm_resources_init(const char* sfifo_name,  const char* pfifo_name, pcommon_res com_res){

//Here is shared memory initialization

com_res->shm_id=shmget(SHM_KEY, 1024*1024, IPC_CREAT|00777);
if(com_res->shm_id==-1){

    printf("It's a pity,  but I can NOT CREATE a SHM segment because %d\n", errno);
    close(com_res->sfile);
		munmap(com_res->msfile_addr, sf.st_size);
		return -1;
}

com_res->shm_addr=shmat(com_res->shm_id,NULL , 0);
if(com_res->shm_addr==(void*)-1){

    printf("It's a pity,  but I can NOT MAP a SHM segment because %d\n", errno);
    close(com_res->sfile);
		munmap(com_res->msfile_addr, sf.st_size);
		return -1;

}
if(mkfifo(sfifo_name, 00777)==-1){

	if(errno!=EEXIST){
    printf("It's a pity,  but I can NOT CREATE a SFIFO because %d\n", errno);
    close(com_res->sfile);
		munmap(com_res->msfile_addr, sf.st_size);
		shmdt(com_res->shm_addr);
		return -1;
	}
}


if(mkfifo(pfifo_name, 00777)==-1){
	if(errno!=EEXIST){
    printf("It's a pity,  but I can NOT CREATE a PFIFO because %d\n", errno);
    close(com_res->sfile);
		munmap(com_res->msfile_addr, sf.st_size);
		shmdt(com_res->shm_addr);

		return -1;
	}

}
if(sem_init(&com_res->src_sem, 0, 0)==-1){

    printf("It's a pity,  but I can NOT CREATE an SRC_SEM  because %d\n", errno);
    close(com_res->sfile);
		munmap(com_res->msfile_addr, sf.st_size);
		shmdt(com_res->shm_addr);
		return -1;

}
return 0;
}

