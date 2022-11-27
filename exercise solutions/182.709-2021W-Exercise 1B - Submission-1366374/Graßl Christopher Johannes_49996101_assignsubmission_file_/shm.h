/** 
*@file: 	shm.h
*@author:	Christopher Graßl (11816883)
*@date:		14.11.2021
*
*@brief:	This header contains types declarations of functions
*			for the generator and supervisor to work with a
*			shared memory object
*@details:	implementation of the functions in shm.c
*/
#ifndef SHM_H
#define SHM_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "graphs.h"
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>



//Buffer with solutions, read/writePos and a bool variable to initiate termination of the generators
typedef struct buffer{
	solution_t solutions[10]; 	/**< Array of solutions*/
	int writePos;				/**< position to write to*/
	int readPos;				/**< position to read from*/
	bool done;					/**< termination indicator*/
}buffer_t;


/** 
*create a shared memory [shm] object 
*@brief: 	Function to create (server) a shared memory object
* 			and mapping
*@details:	Function can be given the address of a pointer and
*			sets it to a shared memory object which it creates
*@param:	sol_buffer address of the pointer which points to the shm object afterwards
*@return:	returns -1 on failure and the fileDescriptor on success
*/
int createSHM(buffer_t **sol_buffer);

/**
*open a shm object
*@brief: 	Function to open (client) a shm object and create
*			mapping
*@details: 	Takes the address of a pointer and sets it to
*			a shm object which is already created
*@param:	sol_buffer address of the pointer which points to the shm object afterwards
*@return:	returns -1 on failure and the fileDescriptor on success
*/
int openSHM(buffer_t **sol_buffer);

//int createMapping(int fd, buffer_t *solBuffer);		<--can be removed

/**
*close a shm object
*@brief: 	Function to close and, if server, unlink a shm object
*@details:	Takes the file Descriptor and the pointer of a shm object
*			to unmap, close and (if server) unlink it
*@param:	sol_buffer 	pointer to the shm object
*@param:	fd file 	descriptor to the shm object
*@param:	unlink		variable to indicate whether to unlink or not
*@return:	returns -1 on failure and 0 on success
*/
int closeSHM(buffer_t *sol_buffer, bool unlink, int fd);

/** 
*create a Semaphore
*@brief:	Function to create a Semaphore (as Server)
*@details:	Takes the Name, the initial value an the address
*			of a pointer to create a Semaphore which the 
*			pointer points to afterwards
*@param:	sem 		address of the semaphore pointer
*@param:	semName 	Name of the Semaphore
*@param:	initial 	Initial value of the semaphore
*@return:	returns -1 on failure, 0 on success
*/
int createSem(sem_t **sem, char *semName,unsigned int initial);

/** 
*opens a Semaphore
*@brief:	Function to open a Semaphore (as Client)
*@details:	Takes the Name and the address
*			of a pointer to open a Semaphore which the 
*			pointer points to afterwards
*@param:	sem 		address of the semaphore pointer
*@param:	semName 	Name of the Semaphore
*@return:	returns -1 on failure, 0 on success
*/
int openSem(sem_t **sem, char *semName);

/** 
*closes a Semaphore
*@brief:	Function to close and (if Server) unlink a Semaphore
*@details:	Takes the Name and the address
*			of a pointer to close and(if Server) unlink
*			the Semaphore
*@param:	sem 		pointer to the semaphore
*@param:	semName 	Name of the Semaphore
*@return:	returns -1 on failure, 0 on success
*/
int closeSem(sem_t *sem, char *semName, bool unlink);

#endif