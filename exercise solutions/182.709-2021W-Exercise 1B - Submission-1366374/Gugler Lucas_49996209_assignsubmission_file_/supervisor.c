/**
 * @file supervisor.c
 * @author Lucas Gugler 12019849
 * @date 4.11.2021
 *
 * @brief Module implements functionality of supervisor
 * @details The supervisor sets up the shared memory and semaphores. After the setup, the supervisor waits for solutions submitted by the 
 * registered generator and puts out a new solution if it is better as the current one
 **/

#include "shared_resources.h"

/**name of the invoked program*/
static const char *PROGNAME;
/**circular buffer in shared memory*/
static circular_buffer *buffer = NULL;
/**semaphore for reading*/
static sem_t *semset = NULL;
/**semaphore for writing*/
static sem_t *semunset = NULL;
/**semaphore for one max writer*/
static sem_t *semwrite = NULL;

/**
 * Usage function.
 * @brief Prints an usage message to stderr and ends the program
 * @param prog_name Name of the program.
 **/
static void usage(const char prog_name[])
{
    fprintf(stderr, "[%s] Usage: %s \n", prog_name, prog_name);
    exit(EXIT_FAILURE);
}

/**
 * Signal function
 * @brief Function invoked by SIGINT or SIGTERM
 * @param signal type of signal
**/
static void handle_signal(int signal)
{
    buffer->stop = 1;
}

/** Cleanup function 
 * @brief Frees shared and allocated memory as well assemaphores
 * @details Before termination the cleanup function frees all allocated memories as well as the set up shared memory.
 * Furthermore it guarantees the shutdown of the registered generators.
 */
static void cleanup(void)
{
    if (buffer != NULL)
    {
        buffer->stop = 1;
        if (semunset != NULL)
        {
            //shutdown of generators
            for (int i = 0; i < buffer->numgenerators; i++)
            {
                if (sem_post(semunset) < 0)
                {
                    fprintf(stderr, "[%s]: %s\n", PROGNAME, "posting semaphore failed");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    if (semset != NULL)
    {
        if (sem_close(semset) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "closing semaphore failed");
            exit(EXIT_FAILURE);
        }
        if (sem_unlink(SEMSET) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "unlinking semaphore failed");
            exit(EXIT_FAILURE);
        }
    }
    if (semunset != NULL)
    {
        if (sem_close(semunset) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "closing semaphore failed");
            exit(EXIT_FAILURE);
        }
        if (sem_unlink(SEMUNSET) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "unlinking semaphore failed");
            exit(EXIT_FAILURE);
        }
    }
    if (semwrite != NULL)
    {
        if (sem_close(semwrite) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "closing semaphore failed");
            exit(EXIT_FAILURE);
        }
        if (sem_unlink(SEMWRITE) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "unlinking semaphore failed");
            exit(EXIT_FAILURE);
        }
    }

    if (buffer != NULL)
    {
        buffer->stop = 1;
        if (munmap(buffer, sizeof(*buffer)) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "unmapping shared memory failed");
            exit(EXIT_FAILURE);
        }
    }
    if (shm_unlink(SHMLOCATION) < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "unlinking shared memory failed");
        exit(EXIT_FAILURE);
    }
}

/** 
 * @brief Waits for new solutions and prints them
 * @details This function waits for the generators to write new solutions into the shared memory until the program is terminated or a solution with 0 edges is found.
 * If the new solution is better then the current one, it gets printed to stdout.
 */
static void supervising(void)
{
    // current best solution
    feedback_arc_set bestsol = {.counter = -1};
    // running until termination
    while (buffer->stop == 0)
    {
        if (sem_wait(semset) < 0)
        {
            if (errno == EINTR)
            {
                exit(EXIT_SUCCESS);
            }
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "waiting for semaphore failed");
            exit(EXIT_FAILURE);
        }
        //reading from shared memory
        feedback_arc_set newset = buffer->data[buffer->read_position];
        buffer->read_position = (buffer->read_position + 1) % BUFFER_SIZE;
        if (sem_post(semunset) < 0)
        {

            fprintf(stderr, "[%s]: %s\n", PROGNAME, "setting semaphore failed");
            exit(EXIT_FAILURE);
        }
        if (newset.counter == 0)
        {
            fprintf(stdout, "No Solution! Acyclic graph!\n");
            buffer->stop = 1;
        }
        else if (newset.counter < bestsol.counter || bestsol.counter == -1)
        {
            //printing and saving new best solution
            bestsol = newset;
            fprintf(stdout, "Solution with %d edges:", bestsol.counter);
            for (int i = 0; i < bestsol.counter; i++)
            {
                fprintf(stdout, " %ld-%ld", bestsol.edges[i].u, bestsol.edges[i].v);
            }
            printf("\n");
        }
    }
}

/**
 * Main function
 * @brief This function is the entrypoint of the supervisor program. It implements the setup of resources.
 * @param argc
 * @param argv
 * @details The semaphores and shared memory as well as signal handling and the cleanup function are set up in this function.
 * @return Upon success EXIT_SUCCESS is returned, otherwise EXIT_FAILURE.
 */
int main(int argc, char const *argv[])
{
    PROGNAME = argv[0];
    if (argc != 1)
    {
        usage(argv[0]);
    }
    if (atexit(cleanup) < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "setting up cleanup function failed");
        exit(EXIT_FAILURE);
    }
    //shared memory setup
    int shm = shm_open(SHMLOCATION, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "setting up shared memory failed");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm, sizeof(circular_buffer)) < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "setting size of shared memory failed");
        close(shm);
        exit(EXIT_FAILURE);
    }
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (buffer == MAP_FAILED)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "mapping of shared memory failed");
        close(shm);
        exit(EXIT_FAILURE);
    }
    if (close(shm) < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "closing of shared memory failed");
        exit(EXIT_FAILURE);
    }

    buffer->stop = 0;
    buffer->read_position = 0;
    buffer->write_position = 0;
    buffer->numgenerators = 0;

    //semaphores setup
    semset = sem_open(SEMSET, O_CREAT | O_EXCL, 0600, 0);
    semunset = sem_open(SEMUNSET, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    semwrite = sem_open(SEMWRITE, O_CREAT | O_EXCL, 0600, 1);
    if (semwrite == SEM_FAILED || semset == SEM_FAILED || semunset == SEM_FAILED)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "creating semaphore failed");
        exit(EXIT_FAILURE);
    }
    //signal handling
    struct sigaction sa = {.sa_handler = handle_signal};
    if (sigaction(SIGINT, &sa, NULL) < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "creating signalhandler failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "creating signalhandler failed");
        exit(EXIT_FAILURE);
    }

    supervising();
    exit(EXIT_SUCCESS);
}