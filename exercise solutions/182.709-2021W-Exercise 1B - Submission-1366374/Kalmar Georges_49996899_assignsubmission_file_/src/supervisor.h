/**
 * @file supervisor.h
 * @author Georges Kalmar 9803393
 * @date 11.11.2021
 *
 * @brief Provides some defines and typedef struct definitions for the program.
 * 
 * This header includes the definitions for the Shared Memory and semaphores. The size
 * of the Shared Memory is defined by the ARRSIZE. Also the typedefs for the struct are added. 
 **/

#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h> 	
#include <fcntl.h>
#include <semaphore.h>  	
#include <errno.h>
#include <signal.h>

#define SHM_NAME "/g_shm_3334col"
#define SEM1 "/g_sem1_3334col"
#define SEM2 "/g_sem2_3334col"
#define SEM3 "/g_sem3_3334col"
#define ARRSIZE (50)

typedef struct twoNodes{
	int n1;
	int n2;
}twoNodes_t;

typedef struct twoNodes_arr{
	twoNodes_t arr[8];
	int size;
}twoNodes_arr_t;


#endif