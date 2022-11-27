/**
 * @file supervisor.c
 * @author Branislav Balvan <e12023159@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Supervisor program module for feedback arc set problem.
 * 
 * @details This program supervises parallel-running generators for feedback arc set problem.
 * It always prints out the new found best solution and continues to await new solutions
 * from its generators till it either gets interrupted (SIGTERM or SIGINT) or it finds
 * solution with no edges (acyclic graph).
 **/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/stat.h>

#include "datastructs.h"

char *myprog; /**< The program name.*/
volatile sig_atomic_t quit = 0; /**< Quit variable signalling if the program was interrupted.*/

//Size: 3172 Bytes
/** @brief Struct for shared memory data.*/
struct shm{
	int writePos, quit;
	solution_t buffer[BUFFERSIZE];
};

/**
 * Signal handler.
 * @brief This function is called with the SIGTERM or SIGING signal and 
 * sets the program to terminate.
 */
void handle_signal(int signal){
	quit = 1;
}

/**
 * Mandatory error function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details Global variables: myprog
 */
void error(void){
  fprintf(stderr, "Usage: %s\n", myprog);
  exit(EXIT_FAILURE);
}

/**
 * Main function of the module.
 * @brief In the main section of supervisor program sets up the shared memory, semaphores 
 * and initializes the circular buffer for the communication with the generators.
 * Then waits for solutions.
 * @details The reading & writing functionality is controlled by semaphores and the solutions
 * are read from the circular buffer in the shared memory.
 * global variables: myprog, quit
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv){
	myprog = argv[0];
	
	struct sigaction sai;
	memset(&sai, 0, sizeof(sai));
	sai.sa_handler = handle_signal;
	sigaction(SIGINT, &sai, NULL);
	
	struct sigaction sat;
	memset(&sat, 0, sizeof(sat));
	sat.sa_handler = handle_signal;
	sigaction(SIGTERM, &sat, NULL);
	
	if(argc > 1){
		fprintf(stderr, "This program takes no arguments.\n");
		error();
	}
	
	//INITIALIZE SHARED MEMORY
	//0660 = [rw-rw----]
	int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
    if (shmfd == -1) {
        fprintf(stderr, "[%s] ERROR: shm_open failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmfd, sizeof(struct shm)) < 0) {
        fprintf(stderr, "[%s] ERROR: ftruncate failed: %s\n", myprog, strerror(errno));
        close(shmfd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    
	struct shm *shm_ptr;
	shm_ptr = mmap(NULL, sizeof(*shm_ptr), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	
	if (shm_ptr == MAP_FAILED) {
        fprintf(stderr, "[%s] ERROR: mmap failed: %s\n", myprog, strerror(errno));
        close(shmfd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    
	//INITIALIZE SEMAPHORES
	sem_t *write_sem = sem_open(SEM_1, O_CREAT | O_EXCL, 0660, BUFFERSIZE);
    sem_t *read_sem = sem_open(SEM_2, O_CREAT | O_EXCL, 0660, 0);
    sem_t *gen_sem = sem_open(SEM_3, O_CREAT | O_EXCL, 0660, 1);
    
    //INITIALIZE SOLUTION & CURSOR POSTIONS
    solution_t best;
    best.ecnt = MAXEDGES + 1;

	int readPos = 0;
	shm_ptr->writePos = 0;
    shm_ptr->quit = 0;
	
	//WHILE LOOP BEGIN$
	while(!quit){
		if(sem_wait(read_sem) == -1){
			if(errno == EINTR)
				continue;
			fprintf(stderr, "[%s] ERROR: sem_wait failed: %s\n", myprog, strerror(errno));
			exit(EXIT_FAILURE);
		}
		//READ FROM SHARED MEMORY
		solution_t new = shm_ptr->buffer[readPos];
		readPos = (readPos + 1) % BUFFERSIZE;
		sem_post(write_sem);
		
		//ACYCLIC GRAPH
		if(new.ecnt == 0){
			fprintf(stdout,"[%s] The graph is acyclic!\n", myprog);
			quit = 1;
			break;
		}
		
		//CHECK IF BETTER SOLUTION
		if(new.ecnt < best.ecnt){
			best = new;
			fprintf(stdout,"[%s] Solution with %d edges:", myprog, best.ecnt);
			for(int i = 0; i < best.ecnt; i++){
				fprintf(stdout," %d-%d", best.edges[i].start, best.edges[i].end);
			}
			fprintf(stdout,"\n");
		}
		 
	}
	//WHILE LOOP END$
	
	shm_ptr->quit = 1;
		
	//DEALLOCATE SHARED MEMORY
	if(munmap(shm_ptr, sizeof(*shm_ptr)) == -1 ){
		fprintf(stderr, "[%s] ERROR: munmap failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
	}
	
	if(close(shmfd) == -1 ){
		fprintf(stderr, "[%s] ERROR: close failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
	}
	
	if(shm_unlink(SHM_NAME) == -1 ){
		fprintf(stderr, "[%s] ERROR: shm_unlink failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
	}
	
	//TERMINATE SEMAPHORES
	sem_close(write_sem);
	sem_close(read_sem);
	sem_close(gen_sem);
	
	sem_unlink(SEM_1);
	sem_unlink(SEM_2);
	sem_unlink(SEM_3);
	
	exit(EXIT_SUCCESS);
}
