/**
 * @file supervisor.c
 * @author Stefan Hettinger <e11909446@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief supervisor program module.
 * 
 * @details This program implements part of the first assignment (B) called "fb_arc_set".
 * The supervisor takes no inputs. It initializes all the semaphores, the circular buffer as shared memory
 * and waits for feedback arc set solutions from the generators. The next lowest feedback arc set is printed
 * until a set with zero edges is found (graph is acyclic). Otherwise the program runs indefinitely trying
 * to read lower solutions.
 **/

#include "supervisor.h"

//global variables
static const char *PROGNAME = "undefined";
static circular_buffer *circ_buffer = NULL; //the circular buffer variable
static sem_t *usedSem = NULL, *freeSem = NULL, *busySem = NULL; //the semaphore variables
volatile sig_atomic_t quit = 0;

//List of functions:
static void init_shm(int shm_fd);
static void term_circ_buffer_and_clean(int shm_fd);
static void handle_signal(int sig);
static void usage(char *message);
static void exitFailureErrno(char *msg, char *errno_details);

/**
 * Program entry point.
 * @brief The main function starts the initialization of the shared memory, semaphores and
 * signal handling.
 * @details This function firstly checks that there are no inputs. Then the signal handler
 * is registered. Afterwards the shared memory is initialized, the circular buffer gets
 * prepared and the semaphores are set. Lastly, the while loop checks the circular buffer
 * for possible solutions until terminated.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int main(int argc, char *argv[]) {
    PROGNAME = argv[0];
    int shm_fd = -1;
    edge_array temp_best = { .nr_of_edges = 20 }; //20 because it's higher than '8' from Assignment

    if(argc != 1) //check for no inputs
        usage("Wrong arguments found!");

    //setup signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    if(sigaction(SIGINT, &sa, NULL) < 0) {
        term_circ_buffer_and_clean(shm_fd);
        exitFailureErrno("Could not set signal handler!", strerror(errno));
    }
    if(sigaction(SIGTERM, &sa, NULL) < 0) {
        term_circ_buffer_and_clean(shm_fd);
        exitFailureErrno("Could not set signal handler!", strerror(errno));
    }

    init_shm(shm_fd); //initialize the shared memory

    //prepare circular buffer
    circ_buffer->best = INT_MAX;
    circ_buffer->next_read = 0;
    circ_buffer->next_write = 0;
    circ_buffer->nr_of_gen = 0;
    circ_buffer->term = 0;

    //setup semaphores
    busySem = sem_open(BUSY_SEM, O_CREAT | O_EXCL, 0600, 1);
    if(busySem == SEM_FAILED) {
        term_circ_buffer_and_clean(shm_fd);
        exitFailureErrno("Could not create busySem!", strerror(errno));
    }
    freeSem = sem_open(FREE_SEM, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    if(freeSem == SEM_FAILED) {
        term_circ_buffer_and_clean(shm_fd);
        exitFailureErrno("Could not create freeSem!", strerror(errno));
    }
    usedSem = sem_open(USED_SEM, O_CREAT | O_EXCL, 0600, 0);
    if(usedSem == SEM_FAILED) {
        term_circ_buffer_and_clean(shm_fd);
        exitFailureErrno("Could not create usedSem!", strerror(errno));
    }

    //run until zero-edge solution is found or sighandler triggers
    while(quit == 0) {

        if(sem_wait(usedSem) < 0) { //wait for turn
            if(errno != EINTR) {
                term_circ_buffer_and_clean(shm_fd);
                exitFailureErrno("sem_wait failed!", strerror(errno));
            }
        }

        edge_array possibleSolution = circ_buffer->fb_arc_sets[circ_buffer->next_read]; //read possible solution from buffer
        circ_buffer->next_read = (circ_buffer->next_read+1) % BUFFER_SIZE;

        if(sem_post(freeSem) < 0) { //exit shared memory
            term_circ_buffer_and_clean(shm_fd);
            exitFailureErrno("sem_post failed!", strerror(errno));
        }

        if((possibleSolution.nr_of_edges == 0) && (quit == 0)) { //acyclic graph found
            fprintf(stdout, "The graph is acyclic!\n");
            circ_buffer->term = 1;
            quit = 1;
        } else if((possibleSolution.nr_of_edges < temp_best.nr_of_edges) && (quit == 0)) { //check if better than old

            temp_best = possibleSolution;
            circ_buffer->best = temp_best.nr_of_edges;
            fprintf(stdout, "Solution with %zu edges:", temp_best.nr_of_edges); //ToDo: add space?

            size_t i;
            for(i = 0; i < temp_best.nr_of_edges; i++) {
                fprintf(stdout, " %ld-%ld", temp_best.edges[i].u, temp_best.edges[i].v); //ToDo: add space?
            }
            fprintf(stdout, "\n");
        }
    }
    term_circ_buffer_and_clean(shm_fd);
    exit(EXIT_SUCCESS);
}

/**
 * @brief The function init_shm initializes the shared memory.
 * @details This function opens, resizes and maps the shared memory
 * file descriptor for the circular buffer.
 * @param shm_fd the shared memory file descriptor.
 * @return Returns void, calls EXIT_FAILURE.
 */
static void init_shm(int shm_fd) {
    shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if(shm_fd < 0)
        exitFailureErrno("Shared memory could not be created!", strerror(errno));

    if(ftruncate(shm_fd, sizeof(circular_buffer)) < 0)
        exitFailureErrno("Shared memory could not be resized!", strerror(errno));

    circ_buffer = mmap(NULL, sizeof(*circ_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(circ_buffer == MAP_FAILED)
        exitFailureErrno("Shared memory could not be mapped!", strerror(errno));
    
    if(close(shm_fd) < 0)
        exitFailureErrno("Filedescriptor could not be closed!", strerror(errno));
}

/**
 * @brief The function is used for cleanup.
 * @details This function is used to unmap and unlink the shared memory and to
 * close and unlink all the semaphores.
 * @param shm_fd the shared memory file descriptor.
 * @return Returns void, does cleanup of shm and sem.
 */
static void term_circ_buffer_and_clean(int shm_fd) {

    if(circ_buffer != NULL) {
        circ_buffer->term = 1; //ensure term is set

        if(freeSem != NULL) { //stop waiting gens
            size_t i;
            for(i = 0; i < circ_buffer->nr_of_gen; i++) {
                if(sem_post(freeSem) < 0)
                    exitFailureErrno("sem_post for freeSem failed!", strerror(errno));
            }
        }
    }

    if(munmap(circ_buffer, sizeof(*circ_buffer)) < 0)
        exitFailureErrno("shm could not be unmapped!", strerror(errno));

    if(shm_fd != -1) { //close shm
        if(close(shm_fd) < 0)
            exitFailureErrno("shm_fd could not be closed!", strerror(errno));
    }

    if (shm_unlink(SHM_NAME) < 0) //unlink shm
        exitFailureErrno("shm could not be unlinked!", strerror(errno));

    if(usedSem != NULL) { //close sem and unlink
        if(sem_close(usedSem) < 0)
            exitFailureErrno("usedSem could not be closed!", strerror(errno));

        if(sem_unlink(USED_SEM) < 0)
            exitFailureErrno("usedSem could not be unlinked!", strerror(errno));
    }
    if(freeSem != NULL) {
        if (sem_close(freeSem) < 0)
            exitFailureErrno("freeSem could not be closed!", strerror(errno));

        if (sem_unlink(FREE_SEM) < 0)
            exitFailureErrno("freeSem could not be unlinked!", strerror(errno));
    }
    if(busySem != NULL) {
        if (sem_close(busySem) < 0)
            exitFailureErrno("busySem could not be closed!", strerror(errno));

        if (sem_unlink(BUSY_SEM) < 0)
            exitFailureErrno("busySem could not be unlinked!", strerror(errno));
    }
}

/**
 * @brief The function handle_signal is the function called by the signal handler.
 * @details This function is called whenever SIGTERM or SIGINT is received.
 * @param sig The caught signal.
 * @return Returns void, sets quit to 1 (to stop while loop).
 */
static void handle_signal(int sig) {
    quit = 1;
}

/**
 * @brief The function usage is used to exit the program and write a provided
 * error message and the standard usage command to stderr.
 * @details This function is called whenever unexpected behavor is detected
 * and faulty input is expected to have caused this error.
 * @param msg The provided error message.
 * @return Returns void, calls EXIT_FAILURE.
 */
static void usage(char *msg) {
    fprintf(stderr, "%s ERROR: \"%s\"\n", PROGNAME, msg);
    fprintf(stderr, "Usage: %s\n", PROGNAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief The function exitFailureErrno is used to exit the program and write a provided
 * error message to stderr, including errno information (or even other strings).
 * @details This function is called whenever unexpected behavor is detected. The
 * error message has to be provided manually.
 * @param msg The provided error message.
 * @param errno_details usually strerror(errno) but can take any other string
 * @return Returns void, calls EXIT_FAILURE.
 */
static void exitFailureErrno(char *msg, char *errno_details) {
    fprintf(stderr, "%s ERROR: \"%s (%s)\"\n", PROGNAME, msg, errno_details);
    exit(EXIT_FAILURE);
}
