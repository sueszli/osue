/**
 * @file generator.c
 * @author Branislav Balvan <e12023159@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Generator program module for feedback arc set problem.
 * 
 * @details This program generates solutions for feedback arc set problem from the given edges
 * as non-option program arguments. It generates the solutions till it doesn't get a
 * signal from the parallel-running 'supervisor' program.
 **/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "datastructs.h"

char *myprog; /**< The program name.*/

//Size: 3172 Bytes
/** @brief Struct for shared memory data.*/
struct shm{
	int writePos, quit;
	solution_t buffer[BUFFERSIZE];
};

/**
 * Mandatory error function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details Global variables: myprog
 */
void error(void){
  fprintf(stderr, "Usage: %s [edge...]\n", myprog);
  exit(EXIT_FAILURE);
}

/**
 * @brief This function checks if there are any digits in string.
 * @details Loops through all the characters in string.
 * @param s The string that should be checked.
 * @return Returns number of non-digits.
 */
int containsNonDigits(char *s){
	int cnt = 0;
	for(int i = 0;i < strlen(s);i++){
		if(!isdigit(s[i])){
			cnt++;
		}
	}
	
	return cnt;
}

/**
 * @brief This function gets index of a int in an array.
 * @details Loops through all the elements and searches for int toFind.
 * @param arr Array to be searched.
 * @param toFind Integer to be find.
 * @param size Size of array.
 * @return Returns index of the int if found, else it returns -1.
 */
int getIntInArr(int *arr, int toFind, int size){
	for(int i = 0; i < size; i++){
		if(toFind == arr[i]){
			return i;
		}
	}
	return -1;
}

/**
 * @brief This function shuffles the array (Fish-Yates).
 * @details Loops through all the elements and shuffles them.
 * @param arr Array to be shuffled.
 * @param size Size of array.
 */
void shuffleArray(int *arr, int size){
    for (int i = size-1; i > 0; i--){
        // Pick a random index from 0 to i
        int j = rand() % (i+1);
 
        // Swap arr[i] with the element at random index
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

/**
 * Main function of the module.
 * @brief The main section of generator program the arguments gets parsed as edges and the 
 * graph gets constructed for solution generation.
 * @details First the edges are parsed from the arguments, then the amount of vertices is
 * calculated. Then the vertices get permutated and with every permutation a new solution is generated.
 * The writing functionality is controlled by semaphores and the solutions are read from the 
 * circular buffer in the shared memory.
 * global variables: myprog
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv){
	myprog = argv[0];
		
	srand(time(NULL));
	int opt = 0;
	
	//GET PROGRAM ARGUMENTS
	while((opt = getopt(argc, argv, ":")) != -1){
	    switch (opt){
	      case ':':
	        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	        error();
	      case '?':
	       if (isprint(optopt)){
	          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
	          error();
	        }else{
	          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
	          error();
	        }
	    }
	}
	
	//ERROR: NO PROGRAM ARGUMENTS
	if(argc <= 1){
		fprintf(stderr, "At least one edge needed as non-option argument!\n");
		error();
	}
		
	//INITIALIZE SHARED MEMORY
	//0660 = [rw-rw----]
	int shmfd = shm_open(SHM_NAME, O_RDWR , 0660);
    if (shmfd == -1) {
        fprintf(stderr, "[%s] ERROR: shm_open failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
	struct shm *shm_ptr;
	shm_ptr = mmap(NULL, sizeof(*shm_ptr), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	
	if (shm_ptr == MAP_FAILED) {
        fprintf(stderr, "[%s] ERROR: mmap failed: %s\n", myprog, strerror(errno));
        close(shmfd);
        exit(EXIT_FAILURE);
    }
	
	//INITIALIZE SEMAPHORES
	sem_t *write_sem = sem_open(SEM_1, 0);
    sem_t *read_sem = sem_open(SEM_2, 0);
    sem_t *gen_sem = sem_open(SEM_3, 0);
	
	//INIT EDGE DATA STRUCTURE
	edge_t *edges = malloc((argc-1) * sizeof(edge_t));
	if(edges == NULL){
		fprintf(stderr, "[%s] ERROR: malloc failed: %s\n", myprog, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	int ecnt = 0;
	//PARSE ALL THE EDGES
	while(optind < argc){
		char *c = argv[optind];
		//printf("%s\n", argv[optind]);
		
		char *t = strtok(c,"-");
		edge_t e;
		
		int cnt = 0;
		while(t != NULL){
			//Error: Is vertex a number?
			if(containsNonDigits(t) > 0){
				fprintf(stderr, "Edge `[start]' and `[end]' have to be a number.\n");
				error();
			}
			switch (cnt){
				case 0:
					e.start = strtol(t, 0, 10);
					break;
				case 1:
					e.end = strtol(t, 0, 10);
					break;
				default:
					//Error: The length of the edge argument is not correct.
					fprintf(stderr, "Edges have to be defined as `[start]-[end]'.\n");
					error();
			}
			t = strtok (NULL, "-");
			cnt++;
		}
		
		if(cnt != 2){
			//Error: The length of the edge argument is not correct.
			fprintf(stderr, "Edges have to be defined as `[start]-[end]'.\n");
			error();
		}
		
		//printf("%d-%d\n", e.start, e.end);
		edges[ecnt] = e;
		
		optind++;
		ecnt++;
	}
	
	//EXTRACT ALL VERTICES FROM EDGES
	int *vtmp = malloc(sizeof(int) * (ecnt * 2));
	if(vtmp == NULL){
		fprintf(stderr, "[%s] ERROR: malloc failed: %s\n", myprog, strerror(errno));
		free(edges);
		exit(EXIT_FAILURE);
	}
	
	int vcnt = 0;
	for(int i = 0; i < ecnt; i++){
		if(getIntInArr(vtmp, edges[i].start, vcnt) == -1){
			vtmp[vcnt] = edges[i].start;
			vcnt++;
		}
		
		if(getIntInArr(vtmp, edges[i].end, vcnt) == -1){
			vtmp[vcnt] = edges[i].end;
			vcnt++;
		}
	}
		
	//INIT VERTEX DATA STRUCTURE
	int *vertices = realloc(vtmp, sizeof(int) * vcnt);
	if(vertices == NULL){
		free(edges);
		free(vtmp);
		fprintf(stderr, "[%s] ERROR: realloc failed: %s\n", myprog, strerror(errno));
		exit(EXIT_FAILURE);
	}
		
	edge_t *fbptr = malloc(ecnt * sizeof(edge_t));
	if(fbptr == NULL){
		free(edges);
		free(vertices);
		fprintf(stderr, "[%s] ERROR: malloc failed: %s\n", myprog, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//WHILE LOOP BEGIN$
	while(!shm_ptr->quit){
		//RANDOM PERMUTATION
		shuffleArray(vertices,vcnt);
		
		//GET FB ARC SET
		int fbcnt = 0;
		for(int i = 0; i < ecnt; i++){
			int first = getIntInArr(vertices, edges[i].start, vcnt);
			int second = getIntInArr(vertices, edges[i].end, vcnt);
			if(first > second){
				//ADD TO FB SET
				//printf("FB@%d-%d\n", edges[i].start, edges[i].end);
				fbptr[fbcnt] = edges[i];
				fbcnt++;
			}
		}
		
		if(fbcnt <= MAXEDGES){
			//PREPARE SOLUTION FOR SUPERVISOR
			solution_t new;
			for(int i = 0; i < fbcnt; i++){
				new.edges[i] = fbptr[i];
			}
			new.ecnt = fbcnt;
			
			sem_wait(gen_sem);
			if(shm_ptr->quit){
				sem_post(gen_sem);
				break;
			}
			sem_wait(write_sem);
			
			//WRITE TO SHARED MEMORY
			int writePos = shm_ptr->writePos;
			shm_ptr->buffer[writePos] = new;
			shm_ptr->writePos = (writePos + 1) % BUFFERSIZE;
			
			sem_post(read_sem);
			sem_post(gen_sem);
			
		}	
				
	}//WHILE LOOP END$
	
	//FREE HEAP ALLOCATION
	free(fbptr);
	free(vertices);
	free(edges);
		
	//DEALLOCATE SHARED MEMORY
	if(munmap(shm_ptr, sizeof(*shm_ptr)) == -1 ){
		fprintf(stderr, "[%s] ERROR: munmap failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
	}
	
	if(close(shmfd) == -1 ){
		fprintf(stderr, "[%s] ERROR: close failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
	}
	
	//TERMINATE SEMAPHORES
	sem_close(write_sem);
	sem_close(read_sem);
	sem_close(gen_sem);
	
	exit(EXIT_SUCCESS);
}
