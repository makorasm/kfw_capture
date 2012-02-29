/*
 * =====================================================================================
 *
 *       Filename:  sync_sem.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/28/2012 12:48:10 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MAX KORNYUSHIN (MAKOR), makorasm@gmail.com
 *        Company:  myself
 *
 * =====================================================================================
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>

union semun{

	int val;
	struct semid_ds *buf;
	unsigned short *array;

};

int sync_sem_init(int id,  int val){

	union semun s_un;
	s_un.val=val;

	if(semctl(id, 0, SETVAL, s_un)==-1){

		perror("sync sem init");
		return -1;
	}
	return 0;
}


int wait_for_sem(int id){

	struct sembuf s_buff;
	s_buff.sem_num=0;
	s_buff.sem_op=-1;
	s_buff.sem_flg=SEM_UNDO;

	if(semop(id, &s_buff, 1)==-1){

		perror("wait_for_sem");
		return -1;
	}
	return 0;
}

int free_sem(id){

	struct sembuf s_buff;
	s_buff.sem_num=0;
	s_buff.sem_op=1;
	s_buff.sem_flg=SEM_UNDO;

	if(semop(id, &s_buff, 1)==-1){

		perror("free_sem");
		return -1;
	}
	return 0;
}
