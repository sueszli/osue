/**
 * @file supervisor.c
 * @author Andreas Hoessl <e11910612@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Supervisor module.
 *
 * This is the supervisor module for the implementation of a command line tool for finding out if a given graph is 3-colorable. It communicates with one or more generators via semaphores and a shared memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>

#include "util.h"

volatile sig_atomic_t quit = 0; /** <  Initializing quit flag for signal handling. */

/**
 * Usage function
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: myprog
 */
void usage(void) {
    fprintf(stderr, "usage: %s", myprog);
    exit(EXIT_FAILURE);
}
/**
 * Procedure
 * @brief This procedure sets the quit flag to 1.
 * @details When either a SIGINT or SIGTERM signal is received quit is set to 1 and the while loop in the main function stops.
 * global vairables: quit
 */
static void handle_signal(int signal) {
    quit = 1;
}

/**
 * Function
 * @brief This function initializes the shared memory.
 * @details The function opens the shared memory and reserves a portion of the memory. The shared memory is then mapped to the virtual memory of the process.
 * global vairables: shmfd, myshm,
 * @return Returns 0 on succes and -1 on failure
 */
static int init_shm() {
    
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        return -1;
    }
    
    if (ftruncate(shmfd, sizeof(struct myshm)) < 0) {
        return -1;
    }
    
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    
    if (myshm == MAP_FAILED) {
        return -1;
    }
    
    if (close(shmfd) == -1) {
        return -1;
    }
    
    return 0;
}

/**
 * Function
 * @brief This function initializes the semaphores.
 * @details The function opens the semaphores for synchronisation purposes.
 * global vairables: sem_free, sem_used, sem_mutex
 * @return Returns 0 on succes / -1 on failure
 */
static int init_sem() {
    
    sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_DATA);
    if (sem_free == SEM_FAILED) {
        return -1;
    }
    
    sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    if (sem_used == SEM_FAILED) {
        return -1;
    }
    
    sem_mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1);
    if (sem_mutex == SEM_FAILED) {
        return -1;
    }
    
    return 0;
}

/**
 * Function
 * @brief This function counts the edges.
 * @details The function counts the occurences of the '-' character. This gives information about the number of edges of the possible solution.
 * @param edges the input string
 * @return Returns count
 */
static int edge_count(char *edges) {
    int count = 0;
    for (int i = 0; i < strlen(edges); i++) {
        if (edges[i] == '-') {
            count++;
        }
    }
    return count;
}

/**
 * Function
 * @brief This function closes and unlinks the semaphores.
 * @details Before terminating all semaphores are closed and unlinked.
 * global vairables: sem_free, sem_used, sem_mutex
 * @return Returns 0 on succes / -1 on failure
 */
static int close_unlink_sem() {
    
    if (sem_close(sem_free) == -1) {
        return -1;
    }
    
    if (sem_close(sem_used) == -1) {
        return -1;
    }
    
    if (sem_close(sem_mutex) == -1) {
        return -1;
    }
    
    if (sem_unlink(SEM_FREE) == -1) {
        return -1;
    }
    
    if (sem_unlink(SEM_USED) == -1) {
        return -1;
    }
    
    if (sem_unlink(SEM_MUTEX) == -1) {
        return -1;
    }
    
    return 0;
}

/**
 * Program entry point.
 * @brief The main function that handles all parts of the program.
 * @details The main function starts with checking for the right command syntax. Then signal handling, shared memory and semaphores are initalized. After that variables in the shared memory are set with regards to mutal exclusion. The crutial step of reading in from the ring buffer and evaluating the solution is done in the while loop. Finally all resources are cleaned up.
 * global vairables: myprog, sem_free, sem_used, sem_mutex, myshm
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS
 */
int main(int argc, char **argv) {
    
    myprog = argv[0];
    int c;
    
    while ((c = getopt(argc, argv, "")) != -1) {
        usage();
    }
    if (argc != 1) {
        usage();
    }
    
    /**
     * Initializing the signal handling.
     */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    
    if(init_shm() == -1) { /** < Cheking for the succesful initialization of the shared memory. */
        exit_error("failed initializing shared memory");
    }
    
    if(init_sem() == -1) { /** < Cheking for the succesful initialization of the semaphores. */
        exit_error("failed initializing semaphore");
    }
    
    /**
     * First access to the shared memory.
     */
    s_wait(sem_mutex);
    
    myshm->state = 0;
    myshm->wr_pos = 0;
    myshm->rd_pos = 0;
    
    s_post(sem_mutex);
    
    
    char edges_read[ENTRY_SIZE]; /** < The char pointer used for storing possible solutions as they are read in. */
    char solution[ENTRY_SIZE]; /** < The char pointer with the best solution so far. */
    memset(solution, '\0', ENTRY_SIZE);
    
    while (!quit) {
        
        memset(edges_read, '\0', ENTRY_SIZE);
        
        /**
         * Reading in possible solutions from the ring buffer happens here.
         */
        s_wait(sem_used);
        
        strncpy(edges_read, myshm->data[myshm->rd_pos], ENTRY_SIZE);
        myshm->rd_pos = (myshm->rd_pos + 1) % MAX_DATA;
        
        s_post(sem_free);
        
        if (strlen(edges_read) == 0) { /** < The graph is 3-colorable if no edges are parsed. */
            printf("The graph is 3-colorable! \n");
            break;
        }
        
        if (strlen(solution) == 0) { /** < The first solution is written here.*/
            strncpy(solution, edges_read, strlen(edges_read));
            printf("Solution with %d edges: %s \n", edge_count(solution), solution);
        }
        
        if (edge_count(edges_read) < edge_count(solution)) { /** < Every solution which is better than the current one will be written here. */
            memset(solution, '\0', ENTRY_SIZE);
            strncpy(solution, edges_read, strlen(edges_read));
            printf("Solution with %d edges: %s \n", edge_count(solution), solution);
        }
        
    }
    myshm->state = -1; /** < Signaling generators the termination of the process. */
    
    /**
     * Cleanup of the resources.
     */
    if (munmap(myshm, sizeof(*myshm)) == -1) {
        exit_error("munmap failed");
    }
    
    if (shm_unlink(SHM_NAME) == -1) {
        exit_error("shm_unlink failed");
    }
    
    if (close_unlink_sem() == -1) {
        exit_error("failed closing/unlinking semaphore");
    }
    
    exit(EXIT_SUCCESS);
}



