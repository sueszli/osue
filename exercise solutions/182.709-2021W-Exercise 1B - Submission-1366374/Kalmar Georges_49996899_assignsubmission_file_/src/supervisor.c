/**
 * @file supervisor.c
 * @author Georges Kalmar 9803393
 * @date 11.11.2021
 *
 * @brief Supervisor main module.
 * 
 * This program takes care of creating shared memory and semaphores, handling signals to stop all processes and printing the best
 * solutions for possible three colorable graphes. 
 **/
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

#include "supervisor.h"

struct myshm{
	unsigned int state;
	unsigned int pos;
	twoNodes_arr_t data[ARRSIZE];
};
/* this global variable is needed for the signal handler*/
static volatile sig_atomic_t quit = 0;

/** 
 * @brief Changes a variable if signal is received
 * @details Sets global varialbe quit to 1 if one of the two specified signals is received
 * @param signal number of signal
 **/
static void handle_signal(int signal){
	quit = 1;
}

/** 
 * @brief Creates shared memory and semaphores, handling signals to stop all processes and printing the best three colorable solution
 * @details Creates Shared Memory and semaphores (and later closed and unlinked/unmapped accordingly) and in a while loop different solutions for 
 * three colored graphes are received from an array in the Shared Memory. If a three colored graph where no edge has to be removed is found or 
 * if a SIGTERM or SIGINT signal is received it is reported to a variable in the Shared Memory and so all processes are closed.
 * @param argc Stores the amount of arguments
 * @param argv Stores the text string of the arguments in an array
 * @return The program return EXIT_SUCCESS on success or returns EXIT_FAILURE in case of errors 
 **/
int main(int argc, char* argv[]){
	
	//open shared memory and set size
	int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
	if(shmfd == -1){
		fprintf(stderr,"%s Error on opening or creating shared memory: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(ftruncate(shmfd,sizeof(struct myshm)) < 0){
		fprintf(stderr,"%s Error on ftruncate: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	struct myshm *myshm;
	myshm = mmap(NULL,sizeof(*myshm),PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if(myshm==MAP_FAILED){
		fprintf(stderr,"%s Error on mmap: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//open/create semaphores
	sem_t *s1 = sem_open(SEM1, O_CREAT | O_EXCL, 0600,ARRSIZE);
	sem_t *s2 = sem_open(SEM2, O_CREAT | O_EXCL, 0600,0);
	sem_t *s3 = sem_open(SEM3, O_CREAT | O_EXCL, 0600,1);
	if((s1 == SEM_FAILED) || (s2 == SEM_FAILED) || (s3 == SEM_FAILED)){
		fprintf(stderr,"%s Error on opening semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//set the signals
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = handle_signal;
	if((sigaction(SIGINT,&sa,NULL)) == -1){
		fprintf(stderr,"%s Error on sigaction: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if((sigaction(SIGTERM,&sa,NULL)) == -1){
		fprintf(stderr,"%s Error on sigaction: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
		
	
	myshm->pos=0; 	//initializes the array position to 0
	myshm->state=0;	//initializes the state to 0, if state == 1 finish all processes
	int i = 0;
	int best_size = 9;
	//coordinates the semaphores and manages the stop of all processes  
	while (1){			
		if(sem_wait(s2) == -1){
			if(errno !=EINTR){
				fprintf(stderr,"%s Error on sem_wait: %s\n",argv[0],strerror(errno));
				exit(EXIT_FAILURE);
			}
		}	
		if(myshm->data[i].size < best_size && quit !=1){
			best_size = myshm->data[i].size;
			if(best_size == 0){
				printf("%s: The graph is 3-colorable!\n",argv[0]);
				myshm->state=1;
			}
			else{printf("%s: Solution with %d edges:",argv[0],best_size);}
			for(int j = 0; j<(best_size);j++){
				printf(" %d-%d",myshm->data[i].arr[j].n1,myshm->data[i].arr[j].n2);
			}
			printf("\n");
		}
		i++;
		i %=ARRSIZE;
		if(sem_post(s1) == -1){
			if(errno !=EINTR){
				fprintf(stderr,"%s Error on sem_post: %s\n",argv[0],strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		if(quit == 1){myshm->state=1;break;}
		if(myshm->state == 1){break;}
	}
	
	//sem close and unlink
	if(sem_close(s1) == -1){
		fprintf(stderr,"%s Error on closing semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	} 
	if(sem_close(s2) == -1){
		fprintf(stderr,"%s Error on closing semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_close(s3) == -1){
		fprintf(stderr,"%s Error on closing semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_unlink(SEM1) == -1){
		fprintf(stderr,"%s Error on unlinking semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_unlink(SEM2) == -1){
		fprintf(stderr,"%s Error on unlinking semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_unlink(SEM3) == -1){
		fprintf(stderr,"%s Error on unlinking semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//close and unlink shared memory and unmap
	if(close(shmfd) == -1){
		fprintf(stderr,"%s Error on closing Shared Memory: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(munmap(myshm,sizeof(*myshm)) == -1){
		fprintf(stderr,"%s Error on munmap: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(shm_unlink(SHM_NAME) == -1){
		fprintf(stderr,"%s Error on shm unlink: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}

}