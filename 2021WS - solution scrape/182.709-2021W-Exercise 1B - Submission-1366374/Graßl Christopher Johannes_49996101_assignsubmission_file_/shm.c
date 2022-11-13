/** 
*@file: 	shm.c
*@author:	Christopher Graßl (11816883)
*@date:		14.11.2021
*
*@brief:	This file contains implementations of functions
*			for the generator and supervisor to work with a
*			shared memory object
*@details:	associated header with datatypes etc.: shm.h
*/


#include "shm.h"

/**
*@brief:	creates shm object
*@details:	elaborate documentation in shm.h
*/
int createSHM(buffer_t **sol_buffer){
	//create sharedMem object
	int shmfd = shm_open("/11816883_buffer", O_RDWR | O_CREAT, S_IRWXU);
	if(shmfd == -1){
		return -1;
	}

	//set the size of the shared mem
	if(ftruncate(shmfd,sizeof(buffer_t)) == -1){
		close(shmfd);
		return -1;
	}

	//create mapping
	*sol_buffer = mmap(NULL,sizeof(buffer_t),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);
	if(*sol_buffer == MAP_FAILED){
		close(shmfd);
		return -1;
	}
	//
	return shmfd;
}

/**
*@brief:	opens shm object
*@details:	elaborate documentation in shm.h
*/
int openSHM(buffer_t **sol_buffer){
	int shmfd = shm_open("/11816883_buffer", O_RDWR, S_IRWXU);
	if(shmfd == -1){
		return -1;
	}
	
	//create mapping
	*sol_buffer = mmap(NULL,sizeof(buffer_t),PROT_READ | PROT_WRITE,MAP_SHARED,shmfd,0);
	if(*sol_buffer == MAP_FAILED){
		close(shmfd);
		return -1;
	}

	return shmfd;
}

/*Create mapping (returns -1 on failure and 0 on success)								/
int createMapping(int fd, buffer_t *solBuffer){											/
	solBuffer = mmap(NULL,sizeof(buffer_t),PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);		/	<--- can be removed
																						/
	if(solBuffer == MAP_FAILED){														/
		return -1;																		/
	}

	return 0;
}*/

/**
*@brief:	closes (and unlinks) shm object
*@details:	elaborate documentation in shm.h
*/
int closeSHM(buffer_t *sol_buffer, bool unlink, int fd){
	if(munmap(sol_buffer,sizeof(buffer_t)) == -1){
		return -1;
	}

	if(close(fd) == -1){
		return -1;
	}

	if(unlink){
		if(shm_unlink("/11816883_buffer") == -1){
			return -1;
		}
	}

	return 0;
}


/**
*@brief:	creates Semaphore
*@details:	elaborate documentation in shm.h
*/
int createSem(sem_t **sem, char *semName,unsigned int initial){
	*sem = sem_open(semName, O_CREAT | O_RDWR, S_IRWXU, initial);

	if(*sem == SEM_FAILED){
		return -1;
	}

	return 0;
}

/**
*@brief:	opens Semaphore
*@details:	elaborate documentation in shm.h
*/
int openSem(sem_t **sem, char *semName){
	*sem = sem_open(semName, O_RDWR);

	if(*sem == SEM_FAILED){
		return -1;
	}

	return 0;
}


/**
*@brief:	closes (and unlinks) semaphore
*@details:	elaborate documentation in shm.h
*/
int closeSem(sem_t *sem, char *semName, bool unlink){
	int ret = sem_close(sem);

	if(unlink){
		ret = sem_unlink(semName);
	}

	return ret;
}