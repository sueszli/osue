/**
 * @file supervisor.c
 * @author Valentin Hrusa 11808205
 * @brief collects Fb_Arc_Sets from generators and prints the best ones
 * @details the supervisor sets up shared memory and semaphores which are used by the generators to
 *          write their solutions to a Feedback Arc Set problem to.
 *          Every solution which contains fewer edges than the previous best solution is printed to the console.
 *          If a solution with 0 Edges is found the programs are terminated gracefully.
 *          If SIGTERM or SIGINT are received the programs are terminate gracefully.
 *          
 * 
 * @date 12.11.2021
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include "structDefs.h"

//global variable for the name of the program
char *name;
//global variable for stopping if signal is received
volatile sig_atomic_t running = 1;
//global variables for the shared memory and semaphores
int shm_fd;
Circular_Buffer *memory;
sem_t* sem_write;
sem_t* sem_read;
sem_t* sem_mutex;

/**
 * @brief called if signal is received
 * @details if SIGTERM or SIGINT is received this function
 *          tells the generators and the supervisor to stop
 * 
 * @param signal representation of signal received
 */
static void handle_signal(int signal);

/**
 * @brief prints a Fb_Arc_Set to console or
 *        notifies the user that the Graph is acyclic
 *
 * @param set Fb_Arc_Set to print
 */
void print_set(Fb_Arc_Set set);



/**
 * @brief opens shared memory and semaphores
 * @details opens shared memory and semaphores. Prints errors if any openings failed
 *          then exits with error. Also initialises the shared memory-values
 */
static void res_open(void);

/**
 * @brief closes shared memory and semaphores
 * @details closes shared memory and semaphores. Prints errors if any closings failed.
 *          Returns an exitcode;
 * 
 * @return int EXIT_SUCCESS if all closings were successful, EXIT_FAILURE otherwise
 */
static int res_close(void);

/**
 * @brief calls sem_wait in controlled environment
 * @details calls sem_wait with passed semaphore and checks if any interruption came from signals.
 *          Exits with error if semaphore failed otherwise
 * 
 * @param sem semaphore to call sem_wait on
 */
static void my_sem_wait(sem_t *sem);

int main(int argc, char **argv){
    name = argv[0];
    //signal-handler setup
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if(argc != 1){
        fprintf(stderr, "ERROR in %s: wrong Usage! Usage: %s\n", name, name);
        exit(EXIT_FAILURE);
    }

    //open shm and semaphore-setup
    res_open();    

    //supervisor-loop
    Fb_Arc_Set temp;
    int index_read = 0;
    while(running){
        my_sem_wait(sem_read);
        temp = memory->buffer[index_read];
        index_read = (index_read + 1) % SHM_MEM_INDEX;
        sem_post(sem_write);
        
        if((temp.count < memory->max_edges) && running){
            memory->max_edges = temp.count;
            print_set(temp);
        }
        if(memory->max_edges == 0){
            memory->exit = 1;
            running = 0;
        }
    }

    //close shm and semaphores
    int EXIT_CODE = res_close();
    exit(EXIT_CODE);
}

void handle_signal(int signal){
    memory->exit = 1;
    running = 0;
}

void print_set(Fb_Arc_Set set){
    size_t i;
    if(set.count == 0){
        fprintf(stdout, "[%s] The graph is acyclic!\n", name);
        return;
    }
    fprintf(stdout, "[%s] Solution with %i edges: ", name, set.count);
    for (i=0; i<set.count; i++){
        fprintf(stdout, "(%i-%i) ", set.edges[i].start_node, set.edges[i].end_node);
    }
    fprintf(stdout, "\n");
}

void res_open(){
    if((shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1){
        fprintf(stderr, "ERROR in %s, Failed to open shared memory\n", name);
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shm_fd, sizeof(Circular_Buffer)) < 0){
        fprintf(stderr, "ERROR in %s, Failed to truncate shared memory\n", name);
        if(shm_unlink(SHM_NAME) == -1) fprintf(stderr, "ERROR in %s, Failed to unlink shared-memory\n", name);
        if(close(shm_fd) == -1) fprintf(stderr, "ERROR in %s, Failed to close shared-memory file-descriptor\n", name);
        exit(EXIT_FAILURE);
    }
    if((memory = (Circular_Buffer*)mmap(NULL, sizeof(Circular_Buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED){
        fprintf(stderr, "ERROR in %s, Failed to map shared memory: %s\n", name, strerror(errno));
        if(shm_unlink(SHM_NAME) == -1) fprintf(stderr, "ERROR in %s, Failed to unlink shared-memory\n", name);
        if(close(shm_fd) == -1) fprintf(stderr, "ERROR in %s, Failed to close shared-memory file-descriptor\n", name);
        exit(EXIT_FAILURE);
    }
    memory->exit = 0;
    memory->max_edges = MAX_ALLOWED_EDGES;
    if((sem_write = sem_open(SEM_WRITE_NAME, O_CREAT | O_EXCL, 0600, SHM_MEM_INDEX)) == SEM_FAILED ||
       (sem_read = sem_open(SEM_READ_NAME, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED ||
       (sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT | O_EXCL, 0600, 1)) == SEM_FAILED)
    {
        if(munmap(memory, sizeof(*memory)) == -1) fprintf(stderr, "ERROR in %s, Failed to unmap shared-memory\n", name);
        if(shm_unlink(SHM_NAME) == -1) fprintf(stderr, "ERROR in %s, Failed to unlink shared-memory\n", name);
        if(close(shm_fd) == -1) fprintf(stderr, "ERROR in %s, Failed to close shared-memory file-descriptor\n", name);
        fprintf(stderr, "ERROR in %s, Failed to open semaphores: %s\n", name, strerror(errno));
        exit(EXIT_FAILURE); 
    }
}

int res_close(){
    int EXIT_CODE = EXIT_SUCCESS;
    //close semaphores
    if (sem_close(sem_write) == -1 ||
		sem_close(sem_read) == -1 ||
		sem_close(sem_mutex) == -1) 
    {
		fprintf(stderr, "ERROR in %s, Failed to close semaphores: %s\n", name, strerror(errno));
        EXIT_CODE = EXIT_FAILURE;
	}
    //unlinking semaphores
    if (sem_unlink(SEM_MUTEX_NAME) < 0 ||
		sem_unlink(SEM_WRITE_NAME) < 0 ||
		sem_unlink(SEM_READ_NAME) < 0) 
    {
        fprintf(stderr, "ERROR in %s, Failed to unlink semaphores: %s\n", name, strerror(errno));
        EXIT_CODE = EXIT_FAILURE;	
    }    
    //close shm
    if(munmap(memory, sizeof(*memory)) == -1){
        fprintf(stderr, "ERROR in %s, Failed to unmap shared memory: %s\n", name, strerror(errno));
        EXIT_CODE = EXIT_FAILURE;
    }
    if(shm_unlink(SHM_NAME) == -1){
        fprintf(stderr, "ERROR in %s, failed to unlink shared memory\n", name);
        EXIT_CODE = EXIT_FAILURE;
    }
    if(close(shm_fd) == -1){
        fprintf(stderr, "ERROR in %s, Failed to close shared-memory file-descriptor\n", name);
        EXIT_CODE = EXIT_FAILURE;
    }
    return EXIT_CODE;
}

void my_sem_wait(sem_t *sem){
    while(running){
        if(sem_wait(sem) == -1){
            if(errno == EINTR){
                continue;
            }else{
                fprintf(stderr, "ERROR in %s, sem_wait() failed with %s\n", name, strerror(errno));
                res_close();
                exit(EXIT_FAILURE);
            }
        }else{
            return;
        }
    }
}