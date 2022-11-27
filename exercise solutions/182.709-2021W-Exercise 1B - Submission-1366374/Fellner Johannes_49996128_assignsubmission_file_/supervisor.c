#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<unistd.h>
#include<semaphore.h>

#define SHARED_MEMORY_NAME "/supervisorshm"
#define SEM_WRITE "/sem_write"
#define SEM_READ "/sem_read"
#define MAX_BUFFER 63

struct vertex{
	int leftNode;
	int rightNode;
};

// size of one buffer_element = 64 Byte
struct buffer_element{
	struct vertex element[8];
};

// running = 1 Byte
// solution = 4 Byte
// write_position = 4 Byte
// read_position = 4 Byte 
// circular_buffer[63] = 63 * 64 = 4032 Byte
// size of the shared memory = 4048 Byte
struct sharedMem{
	bool running;
	int solution;
	int write_position;
	int read_position;
	struct buffer_element circular_buffer[MAX_BUFFER];	
};

int shm;
struct sharedMem *shared_memory;
sem_t *write_sem = -1;
sem_t *read_sem = -1;
volatile sig_atomic_t quit = 0;

/**
 * @brief handles signals like SIGINT and SIGTERM
 * @
 */
static void handle_signal(int signal)
{
	quit = 1;
}

/**
 * @brief prints message of correct synopsis
 */
static void usage(void);


static void usage(void)
{
	(void)fprintf(stderr, "Synopsis:\n\tsupervisor\n");
	exit(EXIT_FAILURE);
}

static void cleanup(void)
{
	// unmap the object from shared memory
	if(munmap(shared_memory, sizeof(*shared_memory)) == -1)
		(void)fprintf(stderr, "Error unmapping object from shared memory\n");
	
	// close the shared memory
	if(close(shm) == -1)
		(void)fprintf(stderr, "Error closing shared memory\n");
	
	// unlink the shared memory
	if(shm_unlink(SHARED_MEMORY_NAME) == -1)
		(void)fprintf(stderr, "Error unlinking shared memory\n");
	
	// close the semaphores
	if(sem_close(write_sem) == -1)
		(void)fprintf(stderr, "Error closing semaphore\n");
	if(sem_close(read_sem) == -1)
		(void)fprintf(stderr, "Error closing semaphore\n");
		
	// unlink the semaphores
	if(sem_unlink(SEM_WRITE) == -1)
		(void)fprintf(stderr, "Error unlinking semaphore\n");
	if(sem_unlink(SEM_READ) == -1)
		(void)fprintf(stderr, "Error unlinking semaphore\n");
	
}

int main (int argc, char **argv)
{
	
	// check if there are no more arguments given to supervisor
	if(argc != 1)
		(void)usage();
		
	// create a new shared memory
	shm = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT, 0600);
	
	if(shm == -1)
		(void)fprintf(stderr, "Error opening shared memory\n");
	
	// set the size of the sructure as size of shared memory
	if(ftruncate(shm, sizeof(struct sharedMem)) < 0)
		(void)fprintf(stderr, "Error setting size of thr shared memory\n");
		
	// map object to shared memory
	shared_memory = mmap(NULL, sizeof(*shared_memory), PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);	
	if(shared_memory == MAP_FAILED)
		(void)fprintf(stderr, "Error mapping object into shared memory\n");

	write_sem = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 0);
	read_sem = sem_open(SEM_READ, O_CREAT | O_EXCL, 0600, MAX_BUFFER);
	
	shared_memory->solution = 3;
	
	int count = 0;
	
	while(quit == 0)
	{
		sleep(5);
	} 
	
	
	(void)cleanup();
}
