/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/21/2012 11:21:50 AM
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

comm_res c_res;
int main(int argc, char* argv[]){
	char c_data[512];
	pipe_cmd p_cmd;
	int w_ind;
	if(init_comm_resource(&c_res)){

		fprintf(stderr, "OMX emu init FAULT!\n");
		exit(-1);
	}
	memset(c_data, 0xAA, 256);
	p_cmd.bf_offset=0;
	p_cmd.bf_size=256;

	for(w_ind =0; w_ind<10; w_ind++){
	
		p_cmd.bf_offset=256*(w_ind % 2);
		*(unsigned*)c_data=w_ind;
		write_comm_data(c_data, &p_cmd, &c_res);
	}
	return 0;
}
