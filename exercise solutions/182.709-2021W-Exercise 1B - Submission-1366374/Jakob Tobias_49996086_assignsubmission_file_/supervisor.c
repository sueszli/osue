/// @file supervisor.c
#include "arcset.h"
#include <signal.h>

/**
 * @brief the supervisor gets results vom the generators and if a number thats smaller than any other numbers was returned, it prints it and the solution to stdout
 * @param no parameters needed
 * @return EXIT_FAILURE on failure else EXIT_SUCCESS
*/
bool noSignal = true;

void handle_signal(int signal)
{
noSignal = false;
}



int main(int argc, char* argv[]) {
	int min = 9;
	int i;
	int returnvalue = EXIT_SUCCESS;
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa)); // initialize sa to 0
	sa.sa_handler = handle_signal;
	sigaction(SIGINT, &sa, NULL);

	// create and/or open the shared memory object:
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1){
    	fprintf(stderr,"error when trying to open filedescriptor of shared memory object\n");
		exit(EXIT_FAILURE);
	}
    

    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct myshm)) < 0){
    	fprintf(stderr,"error when trying to truncate shared memory object\n");
		exit(EXIT_FAILURE);
	}
    
    // map shared memory object:
    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED){
    	fprintf(stderr,"error when trying to map shared memory object\n");
		exit(EXIT_FAILURE);
	}
        
    if ((close(shmfd)) == -1){
    	fprintf(stderr,"error when trying to close shared memory object filedescriptor\n");
		exit(EXIT_FAILURE);
	}
	
	// create and/or open the shared memory object:
    int runningfd = shm_open(RUNNING, O_RDWR | O_CREAT, 0600);
    if (runningfd == -1){
    	fprintf(stderr,"error when trying to open filedescriptor of shared memory object\n");
		exit(EXIT_FAILURE);
	}
    
    // set the size of the shared memory:
    if (ftruncate(runningfd, sizeof(bool)) < 0){
    	fprintf(stderr,"error when trying to truncate shared memory object\n");
		exit(EXIT_FAILURE);
	}
    
    // map shared memory object:
    bool *running;
    running = mmap(NULL, sizeof(bool),PROT_WRITE, MAP_SHARED, runningfd, 0);
    if (running == MAP_FAILED){
    	fprintf(stderr,"error when trying to map shared memory object\n");
		exit(EXIT_FAILURE);
	}
        
    if ((close(runningfd)) == -1){
    	fprintf(stderr,"error when trying to close shared memory object filedescriptor\n");
		exit(EXIT_FAILURE);
	}
	
	*running = true;
	
	// create and/or open the shared memory object:
    int writeposfd = shm_open(WRITE_POS, O_RDWR | O_CREAT, 0600);
    if (writeposfd == -1){
    	fprintf(stderr,"error when trying to open filedescriptor of shared memory object\n");
		exit(EXIT_FAILURE);
	}
    
    // set the size of the shared memory:
    if (ftruncate(writeposfd, sizeof(int)) < 0){
    	fprintf(stderr,"error when trying to truncate shared memory object\n");
		exit(EXIT_FAILURE);
	}
    
    // map shared memory object:
    int *writepos;
    writepos = mmap(NULL, sizeof(int),PROT_WRITE, MAP_SHARED, writeposfd, 0);
    if (running == MAP_FAILED){
    	fprintf(stderr,"error when trying to map shared memory object\n");
		exit(EXIT_FAILURE);
	}
        
    if ((close(writeposfd)) == -1){
    	fprintf(stderr,"error when trying to close shared memory object filedescriptor\n");
		exit(EXIT_FAILURE);
	}
	
	*writepos = 0;
	
	
    sem_t *s1 = sem_open(SEM_MUX, O_CREAT, 0600, 1);
    sem_t *free_sem = sem_open(FREE_SPACE, O_CREAT, 0600, BUF_SIZE);
    sem_t *used_sem = sem_open(USED_SPACE, O_CREAT, 0600, 0);

    if(s1 == SEM_FAILED || free_sem == SEM_FAILED || used_sem == SEM_FAILED){
		fprintf(stderr,"error when trying to create semaphor %s\n",strerror(errno));
		exit(EXIT_FAILURE);
    }

	
	
	
    int rd_pos = 0;
    while (min > 0 && noSignal) {
	while((i = sem_wait(used_sem)) == -1){
		if (errno == EINTR){
			continue;
		} else {
			fprintf(stderr,"error while waiting %s\n",strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
        if(myshm->shmbuffer[rd_pos][0][0] < min){
        	min = myshm->shmbuffer[rd_pos][0][0];
		if(myshm->shmbuffer[rd_pos][0][0] == 0){
			fprintf(stdout,"The graph is acyclic!\n");
		} else {
        		fprintf(stdout,"Solution with %d edges:",myshm->shmbuffer[rd_pos][0][0]);
        		for(i = 1; i < 9;i++){

        			if(myshm->shmbuffer[rd_pos][i][0] < 0){
					break;
				}

        			fprintf(stdout," %d-%d",myshm->shmbuffer[rd_pos][i][0],myshm->shmbuffer[rd_pos][i][1]);
			}
			fprintf(stdout,"\n");
		}
	}
	sem_post(free_sem);
        rd_pos += 1;
        rd_pos %= BUF_SIZE;
    }
    
    *running = false;

        
    // unmap shared memory:
    if ((munmap(myshm, sizeof(*myshm))) == -1){
    	fprintf(stderr,"error when trying to unmap shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
        
    // remove shared memory object:
    if ((shm_unlink(SHM_NAME)) == -1){
    	fprintf(stderr,"error when trying to unlink shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
	
	// unmap shared memory:
    if ((munmap(running, sizeof(bool))) == -1){
    	fprintf(stderr,"error when trying to unmap shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
        
    // remove shared memory object:
    if ((shm_unlink(RUNNING)) == -1){
    	fprintf(stderr,"error when trying to unlink shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
	
	// unmap shared memory:
    if ((munmap(writepos, sizeof(int))) == -1){
    	fprintf(stderr,"error when trying to unmap shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
        
    // remove shared memory object:
    if (shm_unlink(WRITE_POS) == -1){
    	fprintf(stderr,"error when trying to unlink shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
	if((sem_close(s1)) < 0){
	    fprintf(stderr,"error when trying to close semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	if((sem_close(free_sem)) < 0){
	    fprintf(stderr,"error when trying to close semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	if((sem_close(used_sem)) < 0){
	    fprintf(stderr,"error when trying to close semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	if((sem_unlink(SEM_MUX)) < 0){
	    fprintf(stderr,"error when trying to unlink semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	if((sem_unlink(FREE_SPACE)) < 0){
	    fprintf(stderr,"error when trying to unlink semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	if((sem_unlink(USED_SPACE)) < 0){
	    fprintf(stderr,"error when trying to unlink semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	return returnvalue;
}

