/*
 * =====================================================================================
 *
 *       Filename:  videoshmat.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/23/2011 04:36:15 PM
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

int resources_init(const char* sf_name, const char* sfifo_name,  const char* pfifo_name, pcommon_res com_res){

struct stat sf;
	//Here is source file opening and mapping
	com_res->sfile=open(sf_name, O_RDONLY);
	if(com_res->sfile==-1){
    printf("It's a pity,  but I can NOT OPEN this file because %d\n", errno);
		return -1;
	}
if(fstat(com_res->sfile, &sf)==-1){
	
    printf("It's a pity,  but I can NOT GET the file SIZE because %d\n", errno);
    close(com_res->sfile);
		return -1;
}

if(com_res->bf_size==0)com_res->bf_size=1024*1024;

com_res->sf_size=sf.st_size;

if(com_res->bf_size > com_res->sf_size)com_res->bf_size=com_res->sf_size;


com_res->msfile_addr=mmap(NULL,sf.st_size, PROT_READ, MAP_PRIVATE, com_res->sfile, 0 );
if(com_res->msfile_addr==(void*)-1){

  
    printf("It's a pity,  but I can NOT MAP the file because %d\n", errno);
    close(com_res->sfile);
		return -1;

}

printf("Maped file base: %p %d\n", com_res->msfile_addr, sf.st_size);
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

//***************************************************************
void* source_thread(void* prms){

	pcommon_res res=(pcommon_res)prms;
  int fifo_id;

	if((fifo_id=open(res->s_fifoname, O_WRONLY))==-1){
			printf("Source thread cannot open fifo because %d", errno);
			pthread_exit(NULL);
			return NULL;
		
	}
	while(res->sf_offs < res->sf_size){
   
		if(res->sf_size - res->sf_offs < res->bf_size){
			res->bf_size=res->sf_size - res->sf_offs;
		}
		memcpy(res->shm_addr, ((char*)res->msfile_addr)+res->sf_offs, res->bf_size);
		if(write(fifo_id, &res->bf_size, sizeof(res->bf_size))==-1){
			printf("Source thread cannot write to fifo because %d", errno);
			pthread_exit(NULL);
		}
		res->sf_offs+=res->bf_size;

	}

	return NULL;
}


//****************************************************************
//Commands

int cmd_start_cap(pcommon_res p_res){
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if(pthread_create(&p_res->src_thread, &attr, source_thread, p_res)!=0){
	
		printf("Command thread cannot create Source thread because %d", errno);
		return -1;
	
	}

	return 0;
}

int cmd_stop_cap(pcommon_res p_res){

return 0;
}
 
int cmd_frame_complete(pcommon_res p_res){

	return 0;
}
//*****************************************************************
void* command_thread(void* prms){
pcommon_res com_res=(pcommon_res)prms;
pcmd_hdr p_cmd;
int fifo_id;
if((fifo_id=open(com_res->p_fifoname, O_RDONLY))==-1){
	printf("Command Thread cannot open param FIFO because %d\n",errno);
	pthread_exit(NULL);
	return NULL;
}
p_cmd=malloc(4096);

if(!p_cmd){

	printf("Sorry,  Command Thread cannot allocate heap memory!!! By!\n");
	close(fifo_id);
	pthread_exit(NULL);
	return NULL;
}
printf("Hi! I'm COMMAND THREAD!\n");
while(read(fifo_id, p_cmd, 4096)!=0){

switch(p_cmd->cmd){

	case cap_start:
		printf("COMMAND THREAD: Capture start!\n");
		break;
	case cap_stop:
		printf("COMMAND THREAD: Capture stop!\n");
		break;
	default:
		printf("COMMAND THREAD: default!\n");
		break;
}

}
free(p_cmd);
	return NULL;
}
//************************************************ 


//*******************************************************
int main(int argc, char* argv[]){


common_res* res;
int b_size=0;
int ind=0, id;
const char sopt[]="f:s";
char fname[256];	
pthread_attr_t attr;
pthread_t cmd_id;

const struct option lopt[]={
{"fname", required_argument,NULL, 0}, 
{"bufsize", required_argument, NULL, 1}, 
{0, 0, 0 ,0}
};

while(1){

id=getopt_long(argc, argv, sopt, lopt, &ind);

if(id==-1)break;

switch(id){
	case 0:
	case 'f':
		printf("Option %s\n", optarg);
		strncpy(fname, optarg, 256);
		break;
	case 1:
	case 's':
		printf("Option %s\n", optarg);
		b_size=atoi(optarg);
			break;
	default:
		printf("Usage:    \n");
			break;
}

}

res=calloc(sizeof(common_res), 1);

if(!res){
	printf("Sorry,  I've got no heap memory!!! By!\n");
	exit(0);

}

getcwd((char*)res->s_fifoname, 256);
printf("Current directory: %s\n", res->s_fifoname);
strcat(res->s_fifoname, SPNAME);
printf("Current fifo: %s\n",  res->s_fifoname);


getcwd((char*)fname, 256);
printf("Current directory: %s\n",  fname);
strcat(fname, "/s-orig.es");
printf("Current file: %s\n",  fname);

getcwd((char*)res->p_fifoname, 256);
printf("Current directory: %s\n",  res->p_fifoname);
strcat(res->p_fifoname, PPNAME);
printf("Current pfifo: %s\n",  res->p_fifoname);

if(resources_init(fname, res->s_fifoname, res->p_fifoname,res)==-1){

	exit(0);
}

pthread_attr_init(&attr);
if(pthread_create(&cmd_id, &attr, command_thread, res)!=0){

	printf("ERROR: Command thread has not been created because %d", errno);
	free(res);
	exit(-1);
}
pthread_attr_destroy(&attr);

if(pthread_join(cmd_id, NULL)!=0){
	
	printf("ERROR: Command thread has not been joined because %d", errno);
	free(res);
	exit(-1);

}
return 0;
}

