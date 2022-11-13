/**
 * @file supervisor.c
 * @author Fabian Hagmann (12021352)
 * @brief This file contains the solution for the 3coloring-supervisor (execrise1b)
 * @details Synopsis: supervisor<br>
 * The supervisor sets up shared memory (incl. circular buffer) and semaphores. If the
 * circular buffer is filled by generators, the supervisor will process the removed edges
 * and save/print the current best result.<br>
 * The supervisor is furthermore responsible to unlink/cleanup shared memory and semaphores
 * and to ensure that all generators can also terminate.
 * @date 2021-11-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// names of shared memory and semaphores
#define SEM_FREE "/12021352_free"
#define SEM_USED "/12021352_used"
#define SEM_WRITE "/12021352_write"
#define SHM_NAME "/12021352_3coloring"
#define BUFFER_SIZE 8
#define BUFFER_CELL_SIZE 8

// Struct to store one edge (containes 2 vertex id's as Integer)
struct Edge{
    int vertice1id;
    int vertice2id;
};

// Struct to represent one buffer cell (stores up to BUFFER_CELL_SIZE edges)
struct BufferCell{
    struct Edge edges[BUFFER_CELL_SIZE];
};

/* 
 * Struct to represent the shared memory
 * - terminate (0 = run, 1 = terminate) -> implies the generators to stop
 * - rd_/wr_pos [0,7] -> stores the current rd/wr position in the circular buffer
 */
struct SharedMemory{
    int terminate;
    int wr_pos, rd_pos;
    struct BufferCell bufferCells[BUFFER_SIZE];
};

static void usageSupervisor(void);
static void initSharedMemoryAndSemaphores(void);
static void terminateSafe(int signal);
static struct BufferCell readFromCircularBuffer(void);
static int countEgesInBufferCell(struct BufferCell cell);
static void printEdgesInBufferCell(struct BufferCell cell, int edgeCount);
static void cleanupSharedMemoryAndSemaphores(void);

struct SharedMemory *sharedMemory;  // shared memory
sem_t *sem_free;                    // semaphore for free space in circ. buffer
sem_t *sem_used;                    // semaphore for used space in circ. buffer
sem_t *sem_write;                   // semaphore for generators' write permission
char *myprog;                       // program name
int shmfd;                          // filedescriptor for shared memory

/**
 * @brief main function for the supervisor
 * @details main function to ensure synapsis, initialize shared memory and semaphores,
 * read results from circular buffer and close/unlink shared memory and semaphores.
 * @param argc number of given arguments
 * @param argv argument values
 * @return exit code
 */
int main (int argc, char *argv[]) {
    // ensure program is called correctly
    myprog = argv[0];
    char c;
    while ((c = getopt(argc, argv, "")) != -1) {
		switch(c) {
			default:
				usageSupervisor();
		}
	}
    if (argc > 1) {
        usageSupervisor();
    }

    initSharedMemoryAndSemaphores();

    // catch signal sigint
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = terminateSafe;
    sigaction(SIGINT, &sa, NULL);

    // wait for solution until count=0 or SIGINT
    int currentBestCount = BUFFER_SIZE + 1;
    while (currentBestCount != 0) {
        // read, compare and print solution from buffer
        struct BufferCell currentCell = readFromCircularBuffer();
        int currentCount;
        if ((currentCount = countEgesInBufferCell(currentCell)) < currentBestCount) {
            currentBestCount = currentCount;
            printEdgesInBufferCell(currentCell, currentCount);
        }
    }    

    terminateSafe(SIGQUIT);
    exit(EXIT_SUCCESS);
}

/**
 * @brief initializes shared memory and semaphores
 * @details opens, truncates and maps the shared memory. 
 * opens semaphores. Saves necessary data to their correspondig
 * variables in the program. If any step fails, print the
 * corresponding error message and exit with failure
 */
static void initSharedMemoryAndSemaphores(void) {
    // open shm
    shmfd = shm_open(SHM_NAME, O_RDWR | O_EXCL | O_CREAT, 0600); 
    if (shmfd == -1) {
        fprintf(stderr, "[%s] Could not open shared memory\n", myprog);
        exit(EXIT_FAILURE);
    }

    // set size of shm
    if (ftruncate(shmfd, sizeof(*sharedMemory)) < 0) {
        fprintf(stderr, "[%s] Could not set size shared memory\n", myprog);
        exit(EXIT_FAILURE);
    }

    // map shm to object
    sharedMemory = mmap(NULL, sizeof(*sharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (sharedMemory == MAP_FAILED) {
        fprintf(stderr, "[%s] Could not map shared memory to object\n", myprog);
        exit(EXIT_FAILURE);
    }

    // init shared memory values
    sharedMemory->terminate = 0;
    sharedMemory->wr_pos = 0;
    sharedMemory->rd_pos = 0;

     // open semaphore
    sem_free = sem_open(SEM_FREE, O_CREAT, 0600, BUFFER_SIZE);
    sem_used = sem_open(SEM_USED, O_CREAT, 0600, 0);
    sem_write = sem_open(SEM_WRITE, O_CREAT, 0600, 1);
    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED) {
        fprintf(stderr, "[%s] Could not open semaphores\n", myprog);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief read cell from cirular buffer
 * @details wait until used space is not empty, then read the buffer cell
 * and increment the read position
 * @return buffercell that was read from the circular buffer
 */
static struct BufferCell readFromCircularBuffer(void) {
    // wait until circular buffer has new results
    sem_wait(sem_used);
    struct BufferCell val = sharedMemory->bufferCells[sharedMemory->rd_pos];
    sem_post(sem_free);

    sharedMemory->rd_pos += 1;
    sharedMemory->rd_pos %= BUFFER_SIZE;
    return val;
}

/**
 * @brief count the number of edges in the given buffer cell
 * @details interates through the given buffer cell until it reaches
 * the termination edge. Counts the number of edges passed until then
 * and returns it.
 * @param cell buffer cell to be counted
 * @return number of set edges in the buffer cell
 */
static int countEgesInBufferCell(struct BufferCell cell) {
    int i;
    for(i = 0; i < (sizeof(struct BufferCell)/sizeof(struct Edge)); i++) {
        if (cell.edges[i].vertice1id == -1 && cell.edges[i].vertice2id == -1) {
            break;
        }
    }
    return i;
}

/**
 * @brief print a buffer cell to stdout
 * @details prints all set edges in a buffer cell to stdout in the format<br>
 * "Removed Edges(<Number of Edges>): <Vert1>-<Vert2> ..."
 * @param cell buffer cell to be printed
 * @param edgeCount number of set edges in the buffer cell
 */
static void printEdgesInBufferCell(struct BufferCell cell, int edgeCount) {
    int i;
	fprintf(stdout, "Removed Edges(%d)", edgeCount);
	for(i = 0; i < edgeCount; i++) {	
		fprintf(stdout, "%d-%d ", cell.edges[i].vertice1id, cell.edges[i].vertice2id);
	}
    fprintf(stdout, "\n");
}

/**
 * @brief terminate the supervisor safely
 * @details terminate the supervisor. To ensure safe termination of the
 * supervisor and all working generators, set the termination flag, and
 * increase the free semaphore (so that generators can exit their loop).
 * Furthermore cleanup shared memory and semaphores.
 * @param signal sig that caused the termination
 */
static void terminateSafe(int signal) {
    // set terminate flag for generators
    sharedMemory->terminate = 1;
    
    // ensure generators terminate properly
    sem_post(sem_free);

    cleanupSharedMemoryAndSemaphores();
    
    if (signal == SIGINT) {
        exit(EXIT_SUCCESS);   
    }
}

/**
 * @brief cleanup shared memory and semaphores
 * @details unmap, close and unlink shared memory. Close and unlink semaphores
 */
static void cleanupSharedMemoryAndSemaphores(void) {
     // cleanup shm
    if (munmap(sharedMemory, sizeof(*sharedMemory)) == -1) {
        fprintf(stderr, "[%s] could not unmap shared memory\n", myprog);
    }
    if (close(shmfd) == -1) {
        fprintf(stderr, "[%s] could not close shared memory\n", myprog);
    }
    
    // remove shm
    if (shm_unlink(SHM_NAME) == -1) {
        fprintf(stderr, "[%s] could not unlink shared memory\n", myprog);
    };
    
    // close semaphore
    sem_close(sem_free); 
    sem_close(sem_used);
    sem_close(sem_write);
    sem_unlink(SEM_FREE); 
    sem_unlink(SEM_USED); 
    sem_unlink(SEM_WRITE);
}

/**
 * @brief prints usage
 * @details prints usage (as specified in the exercise) to stderr
 */
static void usageSupervisor(void) {
	fprintf(stderr,"Usage %s\n", myprog);
	exit(EXIT_FAILURE);
}