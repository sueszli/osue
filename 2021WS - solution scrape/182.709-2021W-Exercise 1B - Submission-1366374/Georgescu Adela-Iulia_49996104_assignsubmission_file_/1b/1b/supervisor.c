/**
* @author Adela-Iulia Georgescu, 11810894
* @date 02-11-2021
* @brief reads the random 3coloring graph and prints the one with the minimal edges to be removed
* @details generators must be started so they can write in the shared memory
*
**/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>

#define CIRCULARBUFFFERSIZE 516

//global variables
char* myProgram;
volatile sig_atomic_t quit = 0;
int shmfd;
void* memory;
sem_t *spaceSem;
sem_t *usedSem;
sem_t *mutex;

//@brief sets the global value to 1 when the right signal is received
static void handle_signal(int sig);

//@brief returns the number of edges to be removed
int parseSolution(char* solution);

//@brief closes and unmaps the resources used in the program and then exits with exitvalue
static void exit_save(int exitvalue);

//@brief prints an error message
static void error(char* message);

int main(int argc, char *argv[]){
  int sth = 0;
  myProgram = argv[0];

  //initialize signal handling
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa)); 
  sa.sa_handler = *handle_signal;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

	//initialize shared memory
	int shmfd = shm_open("/11810894_shm3col", O_RDWR | O_CREAT, 511);
	if(shmfd<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("shm_open failed");
	}
	sth = ftruncate(shmfd,CIRCULARBUFFFERSIZE+8);
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("ftruncate failed");
	}
	errno = 0;
	memory =	mmap(NULL, CIRCULARBUFFFERSIZE+8, PROT_WRITE | PROT_READ | PROT_EXEC,MAP_SHARED,shmfd,0);
	if(errno!=0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("mmap failed");
	}
	memset(memory,0,CIRCULARBUFFFERSIZE+8);
	
	//set up semaphores
	spaceSem = sem_open("/11810894_space", O_CREAT, 511, CIRCULARBUFFFERSIZE/8);
	if(spaceSem==SEM_FAILED){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_open failed");
	}
	usedSem = sem_open("/11810894_used", O_CREAT, 511,0);
	if(spaceSem==SEM_FAILED){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_open failed");
	}
	mutex = sem_open("/11810894_mutex", O_CREAT, 511,1);
	if(spaceSem==SEM_FAILED){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_open failed");
	}

	//set up variables for supervising
	char* writeAdress = (char*)memory;
	int currentRead=0; //current value to be read
	int currentSolution=0; //current solution
	int minimumEdges = 100000;
	char solution[100];

    while(!quit){

        //if no memory is being usedSem wait
		sth = sem_wait(usedSem);
		if(sth<0){
			if(errno==EINTR){
				break;
			}
      printf("error: %s\n", strerror(errno));
      fflush(stdout);
			error("sem_wait failed");
		}

        //if something has been written, read it
		solution[currentSolution++]=writeAdress[currentRead];	

        //if the symbol last read was the string-terminator, process the solution
		if(writeAdress[currentRead]=='\0'){
            
        	errno = 0;
        	int nrEdges = strtol(solution,NULL,10);	
        	if(errno!=0){
            printf("error: %s\n", strerror(errno));
            fflush(stdout);
        		error("strtol failed");
        	}

			//found the right solution
			if(nrEdges==0){
				printf("The graph is 3-colorable!\n");
				break;
			}
			else if(nrEdges < minimumEdges){
				minimumEdges = nrEdges;
				printf("Solution with %d edges: %s\n",nrEdges,&solution[3]);
			}
			currentSolution=0;
		}

        //increment the current reading position
		currentRead = (currentRead+1) % CIRCULARBUFFFERSIZE;

        //signal that one character has been read, meaning there is one more free spaceSem in the shm
		sth = sem_post(spaceSem);
		if(sth<0){
      printf("error: %s\n", strerror(errno));
      fflush(stdout);
			error("sem_post failed");
		}
	}

	//kill all generator-processes
	int* flag = (int*) memory + CIRCULARBUFFFERSIZE+4;
    *flag = 1;
	//system("killall generator");

    exit_save(EXIT_SUCCESS);
}
void error(char* message){
	fprintf(stderr, "%s: %s\n",myProgram,message);
	fflush(stderr);
	exit_save(EXIT_FAILURE);
}

static void exit_save(int exitvalue){
	
	int sth = 0;

	//clean up the shm
	sth = munmap(memory,CIRCULARBUFFFERSIZE+8);
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("munmap failed");
	}
	sth = close(shmfd);
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("close failed");
	}
	sth = shm_unlink("/11810894_shm3col");
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("shm_unlink failed");
	}

	//clean up the semaphores
	sth = sem_close(spaceSem);
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_close failed");
	}
	sth = sem_close(usedSem);
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_close failed");
	}
	sth = sem_close(mutex);
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_close failed");
	}
	sth = sem_unlink("/11810894_space");
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_unlink failed");
	}
	sth = sem_unlink("/11810894_used");
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_unlink failed");
	}
	sth = sem_unlink("/11810894_mutex");
	if(sth<0){
    printf("error: %s\n", strerror(errno));
    fflush(stdout);
		error("sem_unlink failed");
	}

	exit(exitvalue);
}

static void handle_signal(int sig){
    quit = 1;
}
