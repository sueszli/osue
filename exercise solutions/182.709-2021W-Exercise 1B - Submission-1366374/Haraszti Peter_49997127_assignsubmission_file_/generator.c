/**
 * @file generator.c
 * @author Peter Haraszti <Matriculation Number: 12019844>
 * @date 08.11.2021
 *
 * @brief Finds solutions to the 3coloring problem
 * @details The program takes a graph as an argument. It generates a random 3 coloring for the graph, then it removes all edges that do not comply with the rules of 3 coloring.
 * These edges and the number of those edges are stored in a Solution struct. Only solutions that contain less than 9 edges are accepted.
 * If a Solution is found, that is better than the previous solutions, it is written to a circular buffer in the shared memory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#include "structs.h"

#define SH_NAME_12019844 "/12019844myshm"

#define SEM_FREE_12019844 "/12019844sem_f"
#define SEM_USED_12019844 "/12019844sem_u"
#define SEM_WRITE_12019844 "/12019844sem_write"

int numEdges;
struct myshm *shm;

sem_t *free_sem;
sem_t *used_sem;
sem_t *write_sem;
int wr_pos = 0;

char *programname = NULL;

/**
 * @brief Opens the shared memory
 * @details openSHM opens the shared memory, if no shared memory was created yet, it creates it. In case the operation fails, the program exits with an error. The file descriptor is returned.
 * @param void
 * @return The file descriptor of the shared memory is returned
 */
int openSHM(void) {
    int shmfd = shm_open(SH_NAME_12019844, O_RDWR, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "[%s]: Couldn't open shared memory\n", programname);
        exit(EXIT_FAILURE);
    }
    return shmfd;
}

/**
 * @brief Maps the shared memory
 * @details mapSHM maps the shared memory. If the operation fails, the program exits with an error.
 * @param shmfd: file descriptor of the shared memory to be mapped
 * @return void
 */
void mapSHM(int shmfd) {
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm == MAP_FAILED) {
        fprintf(stderr, "[%s]: shared memory: MAP_FAILED\n", programname);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Unmaps the shared memory
 * @details unmapSHM unmaps the shared memory. If the operation fails, the program exits with an error.
 * @param void
 * @return void
 */
void unmapSHM(void) {
    if (munmap(shm, sizeof(*shm)) == -1) {
        fprintf(stderr, "[%s]: shared memory: munmap failed\n", programname);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Usage Message
 * @details Prints information on how generator should be used. Should be called if the generator is called with unforeseen parameters.
 * The program exits afterwards.
 * @param void
 * @param void
 */
void usage(void) {
    fprintf(stderr, "[%s]: Usage: generator edge...\n", programname);
    exit(EXIT_FAILURE);
}

/**
 * @brief Extracts the first node number from an edge string
 * @details In the input (string), edges are given in the following pattern: Node1-Node2, e.g: 1-2.
 * extractFrom() extracts the first node from the char array and converts it to an integer.
 * @param edge : pointer to a char array, that contains an edge
 * @return The first node of the edge as an integer
 */
int extractFrom(char *edge) {
    int dash = 0;
    while (edge[dash] != '-') {
        dash++;
    }
    char fromStr[dash + 1];
    for (int i = 0; i < dash; i++) {
        fromStr[i] = edge[i];
    }
    fromStr[dash] = 0;
    char *ptr;
    return (int) strtol(fromStr, &ptr, 10);
}

/**
 * @brief Extracts the second node number from an edge string
 * @details In the input (string), edges are given in the following pattern: Node1-Node2, e.g: 1-2.
 * extractFrom() extracts the second node from the char array and converts it to an integer.
 * @param edge : pointer to a char array, that contains an edge
 * @return The second node of the edge as an integer
 */
int extractTo(char *edge) {
    int len;
    for (len = 0; edge[len] != '\0'; ++len);

    int dash = 0;
    while (edge[dash] != '-') {
        dash++;
    }

    char toStr[len - dash + 1];
    for (int i = dash + 1; i < len; i++) {
        toStr[i - (dash + 1)] = edge[i];
    }
    toStr[dash] = 0;

    char *ptr;

    return (int) strtol(toStr, &ptr, 10);
}

/**
 * @brief Main function of generator
 * @details First, the shared memory and the semaphores are opened.
 * Then, the graph is read from the argument and stored in an array of Edge structs.
 * While the supervisor hasn't set the stop flag in the shared memory to 1, a random coloring is generated.
 * Then all invalid edges are removed, those edges are saved in a Solution struct. If the solution has less than 9 edges, and is better than the previous ones, it is written to the circular buffer
 * In the end, the resources are cleaned up.
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS, if the program doesn't exit because of an error
 */
int main(int argc, char **argv) {
    programname = argv[0];
    if (argc == 1) usage();

    // Set up shared memory
    int shmfd = openSHM();
    mapSHM(shmfd);


    // Open semaphores
    free_sem = sem_open(SEM_FREE_12019844, 0);
    used_sem = sem_open(SEM_USED_12019844, 0);
    write_sem = sem_open(SEM_WRITE_12019844, 0);

    // Read graph from command line parameter
    numEdges = argc - 1;
    int vertices = 0;
    struct Edge edges[numEdges];
    for (int i = 1; i < argc; i++) {
        char *edge = NULL;
        edge = argv[i];

        int from = extractFrom(edge);
        if (from > vertices) vertices = from;
        int to = extractTo(edge);
        if (to > vertices) vertices = to;

        struct Edge e;
        e.from = from;
        e.to = to;
        edges[i - 1] = e;
    }

    srand(time(0));
    int minSolution = 8; // we are looking for small solutions. Any solutions that have more than 8 edges will not be accepted

    while (shm->stop != 1) {

        // Create new coloring
        int coloring[vertices + 1];
        for (int i = 0; i <= vertices; i++) {
            coloring[i] = rand() % 3;
        }

        // Remove edges to make graph 3 colorable
        struct Edge removedEdges[8];
        int numRemovedEdges = 0;
        for (int i = 0; i < numEdges; i++) {
            struct Edge ed = edges[i];
            if (coloring[ed.from] == coloring[ed.to]) {
                if (numRemovedEdges < 8) {
                    removedEdges[numRemovedEdges] = ed;
                }
                numRemovedEdges++;
            }
        }

        if (numRemovedEdges < minSolution) { // Valid solution, and smaller than the current best solution
            minSolution = numRemovedEdges;

            // Create Solution struct
            struct Solution solution;
            solution.numEdges = numRemovedEdges;
            for (int i = 0; i < 8; i++) {
                solution.edges[i] = removedEdges[i];
            }

            // Write to buffer
            sem_wait(write_sem);
            sem_wait(free_sem);

            wr_pos = shm->wr_pos;
            shm->buff[wr_pos] = solution;
            //printf("Generator: Wrote to buffer at pos [%d]\n", wr_pos);

            sem_post(used_sem);

            wr_pos += 1;
            wr_pos %= BUFFER_LENGTH;
            shm->wr_pos = wr_pos;
            sem_post(write_sem);


        }

    }

    // Clean up shared memory
    unmapSHM();
    close(shmfd);
    sem_close(free_sem);
    sem_close(used_sem);
    sem_close(write_sem);

    return EXIT_SUCCESS;
}

