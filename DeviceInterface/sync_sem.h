/*
 * =====================================================================================
 *
 *       Filename:  sync_sem.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/28/2012 01:19:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MAX KORNYUSHIN (MAKOR), makorasm@gmail.com
 *        Company:  myself
 *
 * =====================================================================================
 */

#ifndef _SYNC_SEM_
#define _SYNC_SEM_


int sync_sem_init(int id,  int val);
int wait_for_sem(int id);
int free_sem( int id);
#endif

