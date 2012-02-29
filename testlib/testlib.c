/*
 * =====================================================================================
 *
 *       Filename:  testlib.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/21/2012 11:53:38 AM
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
#include "../DeviceInterface/list.h"

int test_callback(void* buff, unsigned bf_size, void* prv_data){

	printf("CALLBACK: buff %p size %d val %d\n", buff, bf_size, *(unsigned*)buff);
	return 0;
}

int main(int argc, char* argv[]){
struct s_ProcessParam param;
pomxpump omx_pump=&param.VideoParam.p_params.omx_pump_params;
call_chain new_entry;
INIT_LIST_HEAD(&omx_pump->callback_chain);
new_entry.read_callback = test_callback;
new_entry.callback_data = NULL;
list_add(&new_entry.entry, &omx_pump->callback_chain );
printf("MAIN LIST: %p %p %p %p\n", &new_entry.entry, new_entry.entry.next, 
		&omx_pump->callback_chain, omx_pump->callback_chain.next);
param.VideoParam.p_type=eOMX_PUMP;
SetProcessParam(&param);
StartProcess();
GetProcessParam(&param);
sem_wait(param.VideoParam.p_params.omx_pump_params.stop_sem);
StopProcess();
	return 0;
}
