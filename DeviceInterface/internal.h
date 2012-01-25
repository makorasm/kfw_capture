/* =====================================================================================
 *
 *       Filename:  internal.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/13/2012 12:42:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MAX KORNYUSHIN (MAKOR), makorasm@gmail.com
 *        Company:  myself
 *
 * =====================================================================================
 */

#ifndef __INTRNAL_PUMP_HEADER__
#define __INTRNAL_PUMP_HEADER__

typedef struct _internal_fpump{

	int				file_size;
	char*			map_addr;
	unsigned	buf_size;
	unsigned	offset;
	pthread_t	thr_id;
	sem_t     stop_sem;
	int				stop_cond;
	pump_callback*	pump_read_callback;//If this parameter is NULL ReadVideoData should be used
	void * 	pump_read_callback_data;

}internal_fpump, *pinternal_fpump;

typedef struct _internal_netpump{


}internal_netpump, *pinternal_netpump;


typedef struct _internal_omxpump{


}internal_omxpump, *pinternal_omxpump;


typedef struct _internal_devpump{


}internal_devpump, *pinternal_devpump;


typedef union _internal_params{
	internal_fpump 		fpump;
	internal_netpump	netpump;
	internal_omxpump  omxpump;
	internal_devpump  devpump;

}internal_params, *pinternal_params;

#endif
