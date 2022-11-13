/**
 * @file generator.c
 * @author Tobias Grantner (12024016)
 * @brief This class implements the logic of a generator that generates random feedback arc sets for the given edges as arguments
 *        and sends them to the supervisor using a shared memory space with a circularbuffer.
 * @date 2021-11-11
 */

#include "shm.h"
#include "graph.h"
#include "circularbuffer.h"
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief sig_atomic_t variable, that is set to true if the program is interrupted by a SIGINT or a SIGTERM signal
 */
volatile sig_atomic_t quit = 0;

/**
 * @brief Handles the interruption of the program by a signal
 * 
 * @param signal The signal that interrupted the program
 */
void handle_signal(int signal)
{
    if(signal == SIGINT || signal == SIGTERM) {
        quit = 1;
    }
}


/**
 * @brief The name of the program. Is set at the beginning of the main-function and used to display the program name in error messages.
 */
static const char *program_name;

/**
 * @brief Displays the given error message after printing the program name.
 * 
 * @param message Error message to be displayed
 */
static void error(const char *message)
{
    fprintf(stderr, "[%s] ERROR: %s\n", program_name, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief Displays the given error message followed by the message from strerror(errno) after printing the program name.
 * 
 * @param message Error message to be displayed
 */
static void error_str(const char *message)
{
    fprintf(stderr, "[%s] ERROR: %s: %s\n", program_name, message, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Is used to parse args to edges.
 * 
 * @param destination Where the parsed edge should be stored in
 * @param arg The arg to be parsed as edge
 * @return int 0 if unsuccessful, 1 if successful
 */
int parse_arg(edge_t * destination, const char * arg) {
    char * first = malloc(sizeof(arg));

    if(first == NULL) return 0;

    strcpy(first, arg);

    char * second = strchr(first, '-');

    if(second == NULL || second != strrchr(first, '-')) {
        free(first);
        return 0;
    }

    *second = '\0';
    second++;

    char * end;

    int x = strtol(first, &end, 10);

    if(*end != '\0') {
        free(first);
        return 0;
    }

    int y = strtol(second, &end, 10);

    if(*end != '\0') {
        free(first);
        return 0;
    }

    *destination = (edge_t) { .x = x, .y = y };

    free(first);
    return 1;
}

/**
 * @brief Creates a random permutation based on the given source.
 * 
 * @param destination Where the permutation should be stored to, needs to be at least count big
 * @param source The integers that should be permutated, needs to be at least count big
 * @param count The number of elements in source that should be permutated
 */
void get_random_permutation(int * destination, const int * source, size_t count) {
    int i;
    for(i = 0; i < count; i++) {
        destination[i] = source[i];
    }

    int j;
    int help;
    for(i = count - 1; i > 0; i--) {
        j = rand() % i;
        help = destination[i];
        destination[i] = destination[j];
        destination[j] = help;
    }
}

/**
 * @brief Simple linear search that returns the index of the value
 * 
 * @param data The data where the value should be searched in
 * @param count The number of elements in data
 * @param value The value that should be searched for
 * @return int Index of the value in data, -1 if not found
 */
int search(const int * data, size_t count, int value) {
    int i;
    for(i = 0; i < count; i++) {
        if(data[i] == value) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Gets the feedback arc set given a set of edges and a set of permutated nodes
 * 
 * @param destination Where the feedback arc set should be stored to
 * @param edges The edges that could be in the feedback arc set
 * @param edgeCount The number of edges in the edges array
 * @param permutation The permutation of nodes from which the feedback arc set is created
 * @param nodeCount The number of nodes in the permutation array
 * @return size_t The size of the feedback arc set
 */
size_t get_feedback_arc_set(edge_t * destination, const edge_t * edges, size_t edgeCount, const int * permutation, size_t nodeCount) {
    size_t fbArcSetCount = 0;

    int i;
    for(i = 0; i < edgeCount; i++)
    {
        int xIndex = search(permutation, nodeCount, edges[i].x);
        int yIndex = search(permutation, nodeCount, edges[i].y);

        if(xIndex >= 0 && yIndex >= 0 && xIndex > yIndex)
        {
            destination[fbArcSetCount] = edges[i];
            fbArcSetCount++;
        } 
    }

    return fbArcSetCount;
}

/**
 * @brief This is the main-function of the program and handles all the errors, parses the arguments, handles shared memory, semaphores and the program logic.
 * 
 * @param argc Number of passed arguments
 * @param argv Passed arguments
 * @return int Exit code
 */
int main(int argc, char **argv)
{
    program_name = argv[0];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);

    srand(time(NULL));

    if (getopt(argc, argv, "") != -1)
    {
        error("This program does not accept anny options, it only accepts edges as parametes in the form of \"n1-n2\", with n1 and n2 being the decimal number of the node.");
    }

    size_t edgeCount = argc - 1;

    edge_t edges[edgeCount];

    size_t fbArcSetCount;
    edge_t fbArcSet[edgeCount];

    int i;
    for(i = 0; i < edgeCount; i++) {
        if(parse_arg(&edges[i], argv[i + 1]) == 0) error("Invalid edge, should be in the form of \"n1-n2\", with n1 and n2 being the decimal number of the node.");
    }

    int nodes[2 * edgeCount];
    size_t nodeCount = get_nodes(nodes, edges, edgeCount);
    int permutation[nodeCount];

    // create and/or open the shared memory object:
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if(shmfd == -1) error_str("Opening shared memory failed");

    // map shared memory object:
    shm_t* shm;
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if(shm == MAP_FAILED) error_str("Shared memory mapping failed");

    sem_t *sem_free = sem_open(CB_SEM_FREE, 0);
    if(sem_free == SEM_FAILED) error_str("Semaphore open failed");

    sem_t *sem_used = sem_open(CB_SEM_USED, 0);
    if(sem_used == SEM_FAILED) error_str("Semaphore open failed");


    while(!quit && !shm->quit)
    {
        get_random_permutation(permutation, nodes, nodeCount);
        fbArcSetCount = get_feedback_arc_set(fbArcSet, edges, edgeCount, permutation, nodeCount);

        if(print_edges(stdout, fbArcSet, fbArcSetCount) < 0) error_str("Printing to stdout failed");
        if(fprintf(stdout, "\n") < 0) error_str("Printing to stdout failed");

        if(fbArcSetCount <= GRAPH_MAX_EDGE_COUNT) {
            if(sem_wait(sem_free) < 0) continue;

            cb_push(&shm->cb, create_graph(fbArcSet, fbArcSetCount));

            if(sem_post(sem_used) < 0) error_str("Unable to post semaphore");
        }
    }


    if (close(shmfd) == -1) error_str("Closing shared memory failed.");

    // unmap shared memory:
    if (munmap(shm, sizeof(*shm)) == -1) error_str("Unmapping shared memory failed");

    if(sem_close(sem_free) < 0) error_str("Closing semaphore failed");
    if(sem_close(sem_used) < 0) error_str("Closing semaphore failed");
    
    return 0;
}
