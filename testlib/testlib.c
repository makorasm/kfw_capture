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


int main(int argc, char* argv[]){
struct s_ProcessParam param;

GetProcessParam(&param);

	return 0;
}
