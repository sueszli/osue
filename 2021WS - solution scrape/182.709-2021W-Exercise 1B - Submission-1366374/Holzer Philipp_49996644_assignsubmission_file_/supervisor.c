/**
 * @file supervisor.c
 * @author Philipp Holzer <e12028208@student.tuwien.ac.at>
 * @date 2021-11-14
 * @brief supervisor
 * @details sets up the shared memory, semaphores and inits the circular buffer (for communicating with the generators)
 *          Waits until the generator writes solutions to it.
 */
#include "fb_arc_set.h"

static void usage(void); 
static void emsg(char* msg, char* details);
static void eexit(char* msg, char* details);

static void handle_signal(int signal);

int main(int argc, char *argv[]);

static void solutions(void);
static void init(void);
static void shutdown(void);

static char *name; // name of the program

static int shm_fd = -1; // shared memory circular buffer file descriptor

static circbuffer *circbuf = NULL; // circular buffer used to write and read solutions

static sem_t *sused = NULL; // space used in the shared buffer, used for signaling the supervisor that it can read data
static sem_t *sfree = NULL; // free space in shared buffer, used for signaling the generator that it can write data
static sem_t *smutex = NULL; // used by generators to ensure mutual exclusion while writing

/**
 * @brief provide correct usage of programm written to stderr and exit program with a EXIT_FAILURE
 * @details global variable: name
 **/
static void usage(void) {
    fprintf(stderr, "USAGE: %s", name);
    exit(EXIT_FAILURE);
}

/**
 * @brief prints given error message to the console
 * @param msg error message
 * @param details of the error
 * @details global variable: name
 **/
static void emsg(char *msg, char *details) {
    fprintf(stderr, "ERROR in [%s]: %s", name, msg);
    if (details != NULL) {
        fprintf(stderr, " (%s)", details);
    }
    fprintf(stderr, "\n");
}

/**
 * @brief prints given error message to the console and exit program with a EXIT_FAILURE
 * @param msg error message
 * @param details of the error
 * @details global variable: name
 **/
static void eexit(char *msg, char *details) {
    emsg(msg, details);
    exit(EXIT_FAILURE);
}

/**
 * @brief inform generators that they should shutdown
 * @param signal unused
 * @details global variable: circbuf
 */
static void handle_signal(int signal) {
    circbuf -> shutdown = 1;
}

/**
 * @brief main function: calling functions that set up shared memory and keep track of best solutions
 * @param argc number of arguments
 * @param argv array of arguments
 * @details global variable: name
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[]) {
    if (argc > 1) {
        usage();
    }
    name = argv[0];

    init();
    solutions();

    return EXIT_SUCCESS; 
}

/**
 * @brief saves the best solution produced by the generators
 * @details global variable: circbuf, sused, sfree, name
 */
static void solutions(void) {
    edgec solution = { .counter = INT_MAX };
    while(circbuf -> shutdown == 0) {
        if(sem_wait(sused) == -1) { // blocks until there is new data to read (in the buffer)
            if (errno == EINTR) continue;
            eexit("while sem_wait for \"sused\"", strerror(errno));
        }

        if (circbuf -> shutdown) { // shutdown program
            exit(EXIT_SUCCESS);
        }

        edgec candidate = circbuf -> data[circbuf -> read]; // read from the circular buffer
        circbuf -> read = (circbuf -> read + 1) % BSIZE; 

        if(sem_post(sfree) == -1) { // informs generators that data has been read and can be overwritten
            eexit("could't increment semaphore", strerror(errno));
        }

        if (candidate.counter == 0) {
            printf("[%s] The graph is acyclic!\n", name);
            circbuf -> shutdown = 1;
        } else if (candidate.counter < solution.counter) {
            solution = candidate;
            printf("[%s] Solution with %d edges:", name, solution.counter);
            for (int i = 0; i < solution.counter; i++) {
                printf(" %d-%d", solution.container[i].f, solution.container[i].t);
            }
            printf("\n");
        }
    }
}

/**
 * @brief creates and maps shared memory, sets signal handler, 
 *        inits shared memory buffer and creates semaphores
 * @details global variables: shm_fd, circbuf, sused, sfree, smutex
 */
static void init(void) {
    shm_fd = shm_open(SHMNAME, O_CREAT | O_RDWR | O_EXCL, 0600); // creates and opens new (or existing) shared memory object
    // O_CREAT = create the shared memory object if it does not exist
    // O_RDWR = open the object for read-write access
    // O_EXCL = if O_CREAT was specified, and a shared memory object with given name already exists => return an error

    if (shm_fd == -1) {
        eexit("unable to create shared memory", strerror(errno));
    }

    if (ftruncate(shm_fd, sizeof(circbuffer)) == -1) {
        eexit("unable to set / change size of shared memory", strerror(errno));
    }

    // map shared memory
    circbuf = mmap(NULL, sizeof(*circbuf), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
    
    if (circbuf == MAP_FAILED) {
        eexit("couldn't map shared memory", strerror(errno));
    }

    if (close(shm_fd) == -1) {
        emsg("couldn't close shared memory \"shm_fd\"", strerror(errno));
    }

    circbuf -> shutdown = 0;
    circbuf -> read = 0;
    circbuf -> write = 0;
    circbuf -> generatorcnt = 0; // init buffer

    // create semaphores or throw errors if unsuccessfull
    sused = sem_open(SUSED, O_CREAT | O_EXCL, 0600, 0);
    if (sused == SEM_FAILED) {
        eexit("could't create semaphore \"sused\"", strerror(errno));
    }

    sfree = sem_open(SFREE, O_CREAT | O_EXCL, 0600, BSIZE);
    if (sfree == SEM_FAILED) {
        eexit("could't create semaphore \"sfree\"", strerror(errno));
    }

    smutex = sem_open(SMUTEX, O_CREAT | O_EXCL, 0600, 1);
    if (smutex == SEM_FAILED) {
        eexit("could't create semaphore \"smutex\"", strerror(errno));
    }

    shm_fd = -1;

    // set signal action handler 
    struct sigaction action;
    action.sa_handler = handle_signal;
    if (sigaction(SIGINT, &action, NULL) == -1 || 
        sigaction(SIGTERM, &action, NULL) == -1) {
        eexit("could't register signal action handler", strerror(errno));
    }

    if(atexit(shutdown) == -1) { // calls cleanup if program is terminated
        eexit("could't register cleanup function", NULL);
    }
}

/**
 * @brief shutdown function
 * @details global variables: shm_fd, circbuf, sused, sfree, smutex
 */
static void shutdown(void) {
    if (circbuf != NULL) {
        circbuf -> shutdown = 1;

        // unlock all waiting generators currently waiting
        if (sfree != NULL) {
            for (int i = 0; i < circbuf -> generatorcnt; i++) {
                if(sem_post(sfree) == -1) {
                    emsg("can't unlock semaphore \"sfree\"", strerror(errno));
                }
            }
        }
    }

    // close shared memory fd (if not already closed)
    if (shm_fd != -1) {
        if (close(shm_fd) == -1) { // close shared memory fd (if not already closed)
            emsg("couldn't close shared memory \"shm_fd\"", strerror(errno));
        }
        shm_fd = -1;
    }

    if (sused != NULL) {
        if (sem_close(sused) == -1) { // close a named semaphore (sused)
            emsg("couldn't close semaphore \"sused\"", strerror(errno));
        }
        if (sem_unlink(SUSED) == -1) { // remove a named semaphore (name = SUSED)
            emsg("couldn't unlink semaphore \"SUSED\"", strerror(errno));
        }
    }

    if (sfree != NULL) {
        if (sem_close(sfree) == -1) { // close a named semaphore (sfree)
            emsg("couldn't close semaphore \"sfree\"", strerror(errno));
        }
        if (sem_unlink(SFREE) == -1) { // remove a named semaphore (name = SFREE)
            emsg("couldn't unlink semaphore \"SFREE\"", strerror(errno));
        }
    }

    if (smutex != NULL) {
        if (sem_close(smutex) == -1) { // close a named semaphore (smutex)
            emsg("couldn't close semaphore \"smutex\"", strerror(errno));
        }
        if (sem_unlink(SMUTEX) == -1) { // remove a named semaphore (name = SMUTEX)
            printf(SMUTEX);
            emsg("couldn't unlink semaphore \"SMUTEX\"", strerror(errno));
        }
    }

    if (circbuf != NULL) {
        circbuf -> shutdown = 1;
        if (munmap(circbuf, sizeof(*circbuf)) == -1) { // unmap shared memory
            emsg("couldn't unmap shared memory", strerror(errno));
        }
    }

    if (shm_unlink(SHMNAME) == -1) { // unlink shared memory
        emsg("couldn't unlinking shared memory \"SHMNAME\"", strerror(errno));
    }
}