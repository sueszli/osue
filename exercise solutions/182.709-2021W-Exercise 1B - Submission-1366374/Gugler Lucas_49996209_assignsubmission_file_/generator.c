/**
 * @file generator.c
 * @author Lucas Gugler 12019849
 * @date 4.11.2021
 *
 * @brief Module implements functionality of generator
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
    fprintf(stderr, "[%s] Usage: %s Edges \n", prog_name, prog_name);
    fprintf(stderr, "[%s] Example: %s 0-1 1-2 2-3 \n", prog_name, prog_name);
    exit(EXIT_FAILURE);
}

/**
 * Signal function
 * @brief Function invoked by SIGINT or SIGTERM
 * @param signal type of signal
**/
static void handle_signal(int signal)
{
    exit(EXIT_SUCCESS);
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
        buffer->numgenerators--;
        if (munmap(buffer, sizeof(*buffer)) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "unmapping shared memory failed");
            exit(EXIT_FAILURE);
        }
    }

    if (semset != NULL)
    {
        if (sem_close(semset) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "closing semaphore failed");
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
    }

    if (semwrite != NULL)
    {
        if (sem_post(semwrite) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "posting semaphore failed");
            exit(EXIT_FAILURE);
        }
        if (sem_close(semwrite) < 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "closing semaphore failed");
            exit(EXIT_FAILURE);
        }
    }
}

/** 
 * @brief Calculates new soltutions according to input graph
 * @param edges edges of input grpah
 * @param numedges number of edges in input graph
 * @param numvertices number of vertices in input graph
 * @details This function generates random permutations of the input vertices. Based on the permutation it calculates a feedback arc set. 
 * If the calculated set has less or equal to 8 edges, the set gets written into the shared memory so the supervisor program can check the soltion.
 * 
 */
static void calculating(edge edges[], long numedges, long numvertices)
{

    while (buffer->stop == 0)
    {
        //generating random permutation
        long ordering[numvertices];

        for (int i = 0; i < numvertices; i++)
        {

            ordering[i] = i;
        }

        //random switching
        for (long i = numvertices - 1; i >= 1; i--)
        {
            long j = random() % (i + 1);
            long temp = ordering[i];
            ordering[i] = ordering[j];
            ordering[j] = temp;
        }

        feedback_arc_set solution;

        solution.counter = 0;
        //checking every edge for feedback arc set condition
        for (long i = 0; i < numedges; i++)
        {

            //getting postion of edge vertices in permutation
            long pos1 = -1;
            long pos2 = -1;
            for (int j = 0; j < numvertices; j++)
            {
                if (ordering[j] == edges[i].u)
                {
                    pos1 = j;
                    break;
                }
            }
            for (int j = 0; j < numvertices; j++)
            {
                if (ordering[j] == edges[i].v)
                {
                    pos2 = j;
                    break;
                }
            }
            //adding edge to solution
            if (pos2 > pos1)
            {
                if (solution.counter < 8)
                {
                    solution.edges[solution.counter] = edges[i];
                    solution.counter = solution.counter + 1;
                }
                else
                {
                    solution.counter = solution.counter + 1;
                    break;
                }
            }
        }
        //writing soltion to circular buffer
        if (solution.counter != 9)
        {
            if (sem_wait(semunset) < 0)
            {
                if (errno == EINTR)
                {
                    exit(EXIT_SUCCESS);
                }
                fprintf(stderr, "[%s]: %s\n", PROGNAME, "waiting for semaphore failed");
                exit(EXIT_FAILURE);
            }
            if (buffer->stop)
            {
                exit(EXIT_SUCCESS);
            }
            if (sem_wait(semwrite))
            {
                if (errno == EINTR)
                {
                    exit(EXIT_SUCCESS);
                }
                fprintf(stderr, "[%s]: %s\n", PROGNAME, "waiting for semaphore failed");
                exit(EXIT_FAILURE);
            }
            buffer->data[buffer->write_position] = solution;
            buffer->write_position = (buffer->write_position + 1) % BUFFER_SIZE;
            if (sem_post(semwrite) < 0)
            {
                fprintf(stderr, "[%s]: %s\n", PROGNAME, "posting semaphore failed");
                exit(EXIT_FAILURE);
            }
            if (sem_post(semset) < 0)
            {
                fprintf(stderr, "[%s]: %s\n", PROGNAME, "posting semaphore failed");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/**
 * Main function
 * @brief This function is the entrypoint of the generator program. It implements the setup of resources.
 * @param argc
 * @param argv
 * @details The semaphores and shared memory as well as signal handling and the cleanup function are set up in this function. 
 * This function also processes the graph input and stores it in different variables 
 * @return Upon success EXIT_SUCCESS is returned, otherwise EXIT_FAILURE.
 */
int main(int argc, char const *argv[])
{
    PROGNAME = argv[0];
    if (argc == 1)
    {
        usage(argv[0]);
    }

    long numedges = argc - 1;
    long numvertices = 0;
    edge edges[argc - 1];
    for (int i = 0; i < argc - 1; i++)
    {
        //spliting up single edge input
        size_t len = strlen(argv[i + 1]);
        char *copy = malloc(len + 1);
        if (copy == NULL)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        strcpy(copy, argv[i + 1]);
        char *char1 = strtok(copy, "-");
        char *char2 = strtok(NULL, "-");
        long ver1 = strtol(char1, NULL, 0);
        if (errno != 0 && ver1 == 0)
        {
            free(copy);
            usage(argv[0]);
        }
        long ver2 = strtol(char2, NULL, 0);
        if (errno != 0 && ver2 == 0)
        {
            free(copy);
            usage(argv[0]);
        }
        if (ver1 == LONG_MIN || ver1 == LONG_MAX || ver2 == LONG_MIN || ver2 == LONG_MAX)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "Vertex number size overflow");
            free(copy);
            exit(EXIT_FAILURE);
        }

        free(copy);
        edge e = {.u = ver1, .v = ver2};
        edges[i] = e;
        //calculating number of vertices
        if (ver1 + 1 > numvertices)
        {
            numvertices = ver1 + 1;
        }
        if (ver2 + 1 > numvertices)
        {
            numvertices = ver2 + 1;
        }
    }

    if (atexit(cleanup) < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "setting up cleanup function failed");
        exit(EXIT_FAILURE);
    }
    //shared memory setup
    int shm = shm_open(SHMLOCATION, O_RDWR, 0600);
    if (shm < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "opening shared memory failed");
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

    //sempahores setup
    semset = sem_open(SEMSET, 0);
    semunset = sem_open(SEMUNSET, 0);
    semwrite = sem_open(SEMWRITE, 0);
    if (semwrite == SEM_FAILED || semset == SEM_FAILED || semunset == SEM_FAILED)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "creating semaphore failed");
        exit(EXIT_FAILURE);
    }

    //signal handling setup
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

    calculating(edges, numedges, numvertices);

    exit(EXIT_SUCCESS);
}