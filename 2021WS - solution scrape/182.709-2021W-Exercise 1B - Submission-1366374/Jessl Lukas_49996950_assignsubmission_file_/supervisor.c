/*
@autor: Jessl Lukas 01604985
@modulename: supervisor.c
@created 08.11.2021

This programm createsa shared memory, which is being written on from another programm called generator. It then reads the written solutions from that shared memory 
and safes the best solutions (if a solution has less edges it is seen as better). If a soltuion with 0 edges is given by the generator, then the input graph is already acylic
and the supervisor will terminate all generators and exit with success. Otherwhise the programm will run in a loop in the attempt to find a solution with 0 edges. */

#include "supervisor.h"

char* shm_name = "1604985_sharedmemory";
struct shm_circular_buffer* shm_cm;

/* @param argc: if argc > 1 an  error occurs.
   @param argv: a string of paremeters given. (only the path to the current file is allowed, any more arguments will make the programm exit with an error).
   
   This programm initializes a shared memory, which will also be used by the generator programm. It runs in a loop and reads the written solutions from the shared memory.
   The best solutions are stored in this function. When a new best solution is found, it will be printed on the stdout.  If a solution with the length of 0 is found, 
   then the programm will tell every generator to shut down, terminating those programms and terminating itself.

   returns EXIT_SUCCESS on getting a solution with 0 edges from our generators. 
*/
int main(int argc, char *argv[]){

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

	if(argc > 1){
		fprintf(stderr, "ERROR @supervisor.c , supervisor has too many Arguments %s.\n", strerror(errno));
		exit(EXIT_FAILURE);	
	}
	
	sem_t* mutex, *sem_read_left_1604985, *sem_write_left_1604985;
	
	int fd = createsharedmemory();
	shm_cm = mapshm(fd);

	if((sem_write_left_1604985 = sem_open("1604985_write_semaphore", O_CREAT | O_EXCL, 0600, CIRCULAR_BUFFER_SIZE)) == SEM_FAILED){
		fprintf(stderr, "ERROR @supervisor.c could not create write semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if((sem_read_left_1604985 = sem_open("1604985_read_semaphore", O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED){
		fprintf(stderr, "ERROR @supervisor.c could not create read semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	if((mutex = sem_open("1604985_mutex", O_CREAT | O_EXCL, 0600, 1)) == SEM_FAILED){
		fprintf(stderr, "ERROR @supervisor.c could not create write semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	shm_cm->best_length = 9;
	shm_cm->generator_stop = 0;
	shm_cm->read_position = 0;
	shm_cm->write_position = 0;

	char* smallest_solution = malloc(sizeof(char) * 8 * shm_cm->best_length);	//gets a size of best_length that are max 9 long, so a pattern of xxx-xxx (as there is a spacebar at the end)

	while(shm_cm->best_length != 0){
		sem_wait(sem_read_left_1604985);
		smallest_solution = readfromcircularbuffer( smallest_solution);
		sem_post(sem_write_left_1604985);
	}
	
	shm_cm->generator_stop = 1;
	
	closesharedmemory(fd, mutex, sem_read_left_1604985, sem_write_left_1604985);

	free(smallest_solution);
	
	return EXIT_SUCCESS;	
}

void signal_handler(int signal){
	shm_cm->generator_stop = 1;

	if(shm_unlink(shm_name)== -1){
		fprintf(stderr, "ERROR @supervisor.c Could unlink Memory %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	if(sem_unlink("1604985_write_semaphore") == -1){
		fprintf(stderr, "ERROR @supervisor.c Could not unlink write semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_unlink("1604985_read_semaphore") == -1){
		fprintf(stderr, "ERROR @supervisor.c Could not unlink read semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_unlink("1604985_mutex") == -1){
		fprintf(stderr, "ERROR @supervisor.c Could not unlink mutex semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

/* Opens the shared memory, and checks for any issues regarding this. 

   @returns the file descriptor for the shared memory.*/
int createsharedmemory(){

	int fd = shm_open(shm_name, O_CREAT| O_RDWR, 0600);									//defines file descriptor for the shared memory
	
	if(fd == -1){																			
		fprintf(stderr, "ERROR @supervisor.c , shared memory could not be created %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if(ftruncate(fd,sizeof(struct shm_circular_buffer))== -1){													//sets size of this shared memory
		fprintf(stderr, "ERROR @supervisor.c Circularbuffer memory could not be allocated %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	return fd;
}

/* @param fd: is the file descriptor for our shared memory.

   Maps the shared memory to our struct.

   @returns a pointer to the struct of our shared memory*/
struct shm_circular_buffer* mapshm(int fd){
	struct shm_circular_buffer* shm_cm = mmap(NULL, sizeof(*shm_cm), PROT_READ | PROT_WRITE, MAP_SHARED, fd , 0);

	if(shm_cm==MAP_FAILED){
		fprintf(stderr, "ERROR @supervisor.c Could not map circular buffer storage %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	return shm_cm;

}

/* @param fd: the file descriptor from our shared memory.
   @param mutex,sem_read_left_1604985, sem_write_left_1604985 are all pointers to our semaphores.

   closes everything and unlinks/unmaps the circular buffer + semaphores from our programm. 
   Essentially it is just the cleanup in a function.

   @returns nothing.*/
void closesharedmemory(int fd, sem_t* mutex,sem_t * sem_read_left_1604985,sem_t* sem_write_left_1604985){
	
	if(munmap(shm_cm,CIRCULAR_BUFFER_SIZE) == -1){
		fprintf(stderr, "ERROR @supervisor.c Error unmap circularbuffer %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(close(fd)==-1){
		fprintf(stderr, "ERROR @supervisor.c Could not close file descriptor %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(shm_unlink(shm_name)== -1){
		fprintf(stderr, "ERROR @supervisor.c Could unlink Memory %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if(sem_close(sem_write_left_1604985) == -1){
		fprintf(stderr, "ERROR @supervisor.c Could not close write semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_close(sem_read_left_1604985) == -1){
		fprintf(stderr, "ERROR @supervisor.c Could not close read semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_close(mutex) == -1){
		fprintf(stderr, "ERROR@supervisor.c Could not close mutex semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_unlink("1604985_write_semaphore") == -1){
		fprintf(stderr, "ERROR @supervisor.c Could not unlink write semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_unlink("1604985_read_semaphore") == -1){
		fprintf(stderr, "ERROR @supervisor.c Could not unlink read semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_unlink("1604985_mutex") == -1){
		fprintf(stderr, "ERROR @supervisor.c Could not unlink mutex semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
}

/* @smallest_solution: points to our current smallest solution.

   This method compares the smallest solution length with the length of a new possible solution from the circular buffer.
   If the length from the new solution is smaller than the current smallest solution, it will be printed on stdout and the new 
   solution will be stored.

   @returns a pointer to our new best solution. */
char* readfromcircularbuffer(char* smallest_solution){
	
	char* possible_smallest_solution = calloc(strlen(shm_cm->circular_buffer[shm_cm->read_position]), sizeof(char)); 
	strcpy(possible_smallest_solution, shm_cm->circular_buffer[shm_cm->read_position]);

	char* solutionedges = calloc (strlen(possible_smallest_solution), sizeof(char));
	int length = 0;

	char* edges = strtok(possible_smallest_solution, " ");
	while(edges != NULL){
		strcat(solutionedges,edges);
		strcat(solutionedges, " ");
		length++;
		edges = strtok(NULL, " ");
	}	
	
	if(length == 0){
		
		printf("This Graph is acyclic, there is no need to remove any nodes\n");
		shm_cm->best_length = 0;
		strcpy(smallest_solution,solutionedges);
		
	} else if(length < shm_cm->best_length ){	
		
		smallest_solution = realloc(smallest_solution, strlen(solutionedges) * sizeof(char));

		strcpy(smallest_solution,solutionedges);
		
		shm_cm->best_length = length;
		
		printf("A new best solution has been found. %d Edges = %s\n", shm_cm->best_length, smallest_solution);
		
	}

	shm_cm->read_position++;
	if(shm_cm->read_position == CIRCULAR_BUFFER_SIZE){
		shm_cm->read_position = 0;
	}


	free(possible_smallest_solution); 
	possible_smallest_solution = NULL;
	free(solutionedges);
	solutionedges = NULL;

	return smallest_solution;
}




