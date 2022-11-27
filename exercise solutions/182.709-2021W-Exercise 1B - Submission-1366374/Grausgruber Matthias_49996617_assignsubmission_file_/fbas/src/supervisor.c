/** Feedback Arc Set  
 *	@file supervisor.c
 *  @author Matthias Grausgruber <e00525708@student.tuwien.ac.at>
 *  @date 13.11.2021
 *  
 *  @brief Supervisor module.
 *
 *	This program reads feedback Arc Sets from the generator module via
 *  a shared memory (created by the supervisor). Semaphores for the
 *  circular buffer are created and initiated by this application.
 *  The supervisor is either terminated in case of a perfect solution
 *  (zero edges) or by signal interrupts. There are no options or
 *  positional arguments.
 *  
 **/

#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define SEM_FS "/00525708_free"
#define SEM_US "/00525708_used"
#define SEM_WR "/00525708_write"
#define SHM_NAME "/00525708_myshm"
#define CB_SIZE (50)
#define E_SIZE (8)

static char *pgm_name; /**< The program name. */
volatile __sig_atomic_t quit = 0; /**< Quit-flag for SIGINT shutdown. */

struct edges { int v1, v2; };
struct circularBuffer {
    int size;
    struct edges e[E_SIZE+1]; 
};
struct myshm {
    int wr_pos, rd_pos, active, shutd;
    struct circularBuffer cb[CB_SIZE];
};

/**
 * Print error and exit.
 * @brief This function prints the error-message to the shell in case of an error and ends the program.
 * @details global variables: pgm_name
 */
void errorExit (int line) {
	fprintf (stderr, "%s: error at line #%d: %s\n", pgm_name, line, strerror(errno));
	exit (EXIT_FAILURE);
}

/**
 * Print error (and no exit).
 * @brief This function writes prints the error-message to the shell in case of an error.
 * 		  In difference to errorExit() it doesn't end the program.
 * @details global variables: pgm_name
 */
void errorPrint (int line) {
	fprintf (stderr, "%s: error at line #%d: %s\n", pgm_name, line, strerror(errno));
}

/**
 * What to do in case of SIG-Interrupt.
 * @brief This function decodes the SIGINT number and starts the application shutdown.
 * @details global variables: quit
 */
void handle_signal (int signal) {
    if (signal == SIGINT) {
        printf ("\n\nSIGINT (%i)! Shutdown Feedback Arc Set.\n\n", signal);
        quit = 1;
    } else if (signal == SIGTERM) {
        printf ("\n\nSIGTERM (%i)! Shutdown Feedback Arc Set.\n\n", signal);
        quit = 1;
    }
    else printf ("\n\nSIGINT code: %i; Signal ignored.\n\n", signal);
}

/**
 * Creation of new shared memory.
 * @brief This function opens a new shared memory SHM_NAME. The application is exited,
 * if there is a equaly named shm. The size of the shm is equal to the struct myshm.
 * @details return values: shmfd
 */
int open_shmem (void) {
    int shmfd = shm_open (SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) errorExit(__LINE__);
    if (ftruncate (shmfd, sizeof(struct myshm)) < 0) errorExit(__LINE__);
    return shmfd;
}

/**
 * Mapping and initialition of shm
 * @brief The shared memory is mapped as r/w and open for joining. Initial values for
 * myshm are set: r/w-position for the circular buffer, state of the shm and a shutdown
 * signal.
 * @details return values: *myshm
 */
struct myshm* init_shmem (int shmfd) {
    struct myshm *myshm = mmap (NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED) errorExit(__LINE__);
    myshm->wr_pos = 0;
    myshm->rd_pos = 0;
    myshm->active = 1;
    myshm->shutd = 0;
    return myshm;
}

/**
 * Program entry point.
 * @brief The main program starts here. This function creates a new shared memory,
 * starts the semaphores and waits for new possible solutions from the generators.
 * The best possible solution will be stored. If the size of the solution reaches zero,
 * the generators get a shutdown signal (best possible solution). In case of SIGINT or
 * SIGTERM the shutdown signal is communicated as well. Befor exiting the application
 * the semaphores and shared memory are being closed and unlinked.
 * 
 * @details Synchronisation with the generators is managed by semaphores (Semaphore API).
 * Data transfer is realized via shared memory. External shutdown has to be a signal
 * interrupt.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main (int argc, char **argv)
{   
    struct circularBuffer sol;    
    sol.size = __INT_MAX__;
    sol.e[0].v1 = -1;
    int size = __INT_MAX__;

    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction (SIGINT, &sa, NULL);

    pgm_name = argv [0];

    /** Shared memory is created and opened here. */
    int shmfd = open_shmem();
    struct myshm *myshm = init_shmem (shmfd);

    /** Semaphores are created here. */
    sem_t *s_free = sem_open (SEM_FS, O_CREAT | O_EXCL, 0600, CB_SIZE);
    if (s_free == SEM_FAILED) errorExit(__LINE__);
    sem_t *s_used = sem_open (SEM_US, O_CREAT | O_EXCL, 0600, 0);
    if (s_used == SEM_FAILED) errorExit(__LINE__);
    sem_t *s_write = sem_open (SEM_WR, O_CREAT | O_EXCL, 0600, 1);
    if (s_write == SEM_FAILED) errorExit(__LINE__);

    /** supervisor is executed as long as the best possible solution (size == 0) isn't found.
     *  Only way to quit is via signal interrupt (quit -> handle_signal()).
     */
    while ((quit == 0) && (sol.size > 0)) {

        /** Synchronised read via semaphores. */
        if (sem_wait (s_used) == -1) errorPrint(__LINE__);
        size = myshm->cb[myshm->rd_pos].size;
        if (size < sol.size) {
            sol = myshm->cb[myshm->rd_pos];
            printf ("Active: %i, Solution Size: %i {", myshm->active, sol.size);
            for (int i=0; (sol.e[i].v1 >= 0) && (i < E_SIZE); i++) printf (" (%i->%i)", sol.e[i].v1, sol.e[i].v2);
            printf (" }\n");
        }
        myshm->rd_pos++;
        myshm->rd_pos %= CB_SIZE;
        if (sem_post (s_free) == -1) errorPrint(__LINE__);

        //printf ("Active: %i, Solution Size: %i {", myshm->active, sol.size);
        //for (int i=0; (sol.e[i].v1 >= 0) && (i < E_SIZE); i++) printf (" (%i->%i)", sol.e[i].v1, sol.e[i].v2);
        //printf (" }\n");
        if (sol.size == 0) printf ("The graph is acyclic!\n");
    }

    /** generators shutdown flag is set. Quit signal reset, if needed for shutdown sequence.
     *  generators may be stuck at sem_wait -> sem_post to kick off the write sequence.
     */
    myshm->shutd = 1;
    quit = 0;
    if (sem_post (s_free) == -1) errorPrint(__LINE__);

    /** If any generator is stuck in shutdown sequence or crashed, another ctrl+c stops the
     *  waiting process.
     */
    printf ("Waiting for generators to quit...");
    while ((myshm->active > 1) && (quit == 0)) {}
    printf ("\nClosing supervisor application.\n");

    /** Close/unlink all semaphores and shared memory. */
    if (close(shmfd) == -1) errorPrint (__LINE__);
    if (munmap (myshm, sizeof (*myshm)) == -1) errorPrint (__LINE__);
    if (shm_unlink (SHM_NAME) == -1) errorPrint (__LINE__);

    if (sem_close (s_free) == -1) errorPrint (__LINE__);
    if (sem_close (s_used) == -1) errorPrint (__LINE__);
    if (sem_close (s_write) == -1) errorPrint (__LINE__);
    if (sem_unlink (SEM_FS) == -1) errorPrint (__LINE__);
    if (sem_unlink (SEM_US) == -1) errorPrint (__LINE__);
    if (sem_unlink (SEM_WR) == -1) errorPrint (__LINE__);

    return EXIT_SUCCESS;
}
