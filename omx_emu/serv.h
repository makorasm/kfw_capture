/*
 * =====================================================================================
 *
 *       Filename:  serv.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/20/2012 11:38:43 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MAX KORNYUSHIN (MAKOR), makorasm@gmail.com
 *        Company:  myself
 *
 * =====================================================================================
 */


typedef struct _comm_res{

	int				sync_pipe_id;
	int				cmd_pipe_id;
	int				shm_id;
	char*			shm_point;
	pthread_t cmd_thread_id;
	int				sync_sem;
	int				callback_sem;
} comm_res, *pcomm_res;


int init_comm_resource(pcomm_res res);

int write_comm_data(char* d_point, ppipe_cmd p_cmd, pcomm_res res);

void* cmd_thread(void* param);
