/**
 * @file generator.c
 * @author Lukas Fink 11911069
 * @date 10.11.2021
 *
 * @brief generator module.
 * 
 * @details a generator reads the edges of a graph 
 * and sends a possible solution to the supervisor.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "datatypes.h"
#include "algorithms.h"
#include "circularBuffer.h"

/**
 * Program entry point.
 * @brief The program starts here. It should be called like './generator 0-1 1-2 ...'.
 * @details TODO
 * -
 * -
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 **/

int main (int argc, char *argv[]) {
    char* programName = argv[0];
    if (argc == 1) {
        fprintf(stderr, "%s: no arguments!\n", programName);
        exit(EXIT_FAILURE);
    }

// read the edges into an array of structs (and check if they have a valid format)
    int eSize = 64;
    struct Edge *edgeArr = malloc(sizeof(struct Edge) * eSize);
    if (edgeArr == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        exit(EXIT_FAILURE);
    }

    int n = 1;
    while (n < argc) {
        if (n >= eSize-1) {
            eSize += 64;
            edgeArr = realloc(edgeArr, sizeof(struct Edge) * eSize);
            if (edgeArr == NULL) {
                fprintf(stderr, "%s: out of memory!\n", programName);
                free(edgeArr);
                exit(EXIT_FAILURE);
            }
        }
        char *input = argv[n];
        struct Edge * inputEdge = constructEdge(input, programName);
        if (inputEdge == NULL) {
            fprintf(stderr, "%s: wrong input Format: '%s'. Should be of form [0..9]+[-][0..9]+. No self-loops!\n", programName, input);
            free(edgeArr);
            exit(EXIT_FAILURE);
        }
        
        edgeArr[n-1] = *inputEdge;
        n++;
    }
    struct Edge nullEdge = {.start=-1, .end=-1};
    edgeArr[n-1] = nullEdge;

// Remove duplicate edges
    struct Edge *withoutDup = removeDuplicateEdges(edgeArr, programName, eSize);
    if (withoutDup == NULL) {
        fprintf(stderr, "%s: removeDuplicateEdges failed!\n", programName);
        free(edgeArr);
        exit(EXIT_FAILURE);
    }
    // edgeArr is no longer used
    free(edgeArr);

// Make a list of vertices
    int *vertices = makeVerticeList(withoutDup);
    if (vertices == NULL) {
        fprintf(stderr, "%s: makeVerticeList failed!\n", programName);
        free(withoutDup);
        exit(EXIT_FAILURE);
    }

// open and map shared memory
    int shmfd = shm_open(SHM_CIRCULAR_BUFFER, O_RDWR, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "%s: shared memory already closed!\n", programName);
        free(withoutDup);
        free(vertices);
        exit(EXIT_FAILURE);
    }

    struct CircularBuffer *circularBuf;
    circularBuf = mmap(NULL, sizeof(*circularBuf), PROT_WRITE, MAP_SHARED, shmfd, 0);

// sem_open(): SEM_FREE, SEM_USED for sync with supervisor + SEM_WRITE for sync between generators
    sem_t *semFree = sem_open(SEM_FREE, 0);
    sem_t *semUsed = sem_open(SEM_USED, 0);
    sem_t *semWrite = sem_open(SEM_WRITE, 0);
    if((semFree == SEM_FAILED) || (semUsed == SEM_FAILED) || (semWrite == SEM_FAILED)){
        fprintf(stderr, "%s: sem_open failed\n", programName);
        free(withoutDup);
        free(vertices);
        if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
            fprintf(stderr, "%s: munmap failed!\n", programName);
        }
        if (close(shmfd) == -1) {
            fprintf(stderr, "%s: close failed!\n", programName);
        }
        if (semFree == SEM_FAILED) {
            if (sem_close(semFree) == -1) {
                fprintf(stderr, "%s: shm_close failed on semFree!\n", programName);
            }
        }
        if (semUsed == SEM_FAILED) {
            if (sem_close(semUsed) == -1) {
                fprintf(stderr, "%s: shm_close failed on semUsed!\n", programName);
            }
        }
        if (semWrite == SEM_FAILED) {
            if (sem_close(semWrite) == -1) {
                fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
            }
        }        
        exit(EXIT_FAILURE);
    }
// seeding rand for random generation of vertice lists
    srand(time(0));
    int terminate = 0;
// ------------------------------- loop begin ------------------------------------  
    while (terminate == 0) {

    // Make a random permutation using fisher_yates_shuffle (return a NEW int array)
        int *random = fisher_yates_shuffle(vertices); 

    // get a feedback arc set from the random permutation using the fas_algorithm (withourDup is not changed!)
        struct FeedbackArc *fbA = fas_algorithm(random, withoutDup, programName);
        
        if (fbA == NULL) { // do not write to the circular buffer, discard the solution!
            free(fbA);
            terminate = circularBuf->terminate;
            if (terminate != 0) {
                break;
            }
        } else {
            terminate = circularBuf->terminate;
            if (terminate != 0) {
                break;
            }
            // write to the circular buffer (MUTEX)
            if (sem_wait(semWrite) == -1) {
                if (errno != EINTR) {
                    fprintf(stderr, "%s: sem_wait: failed!\n", programName);
                    free(withoutDup);
                    free(vertices);
                    free(random);
                    if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
                        fprintf(stderr, "%s: munmap failed!\n", programName);
                    }
                    if (close(shmfd) == -1) {
                        fprintf(stderr, "%s: close failed!\n", programName);
                    }
                    if (sem_close(semFree) == -1) {
                        fprintf(stderr, "%s: shm_close failed on semFree!\n", programName);
                    }
                    if (sem_close(semUsed) == -1) {
                        fprintf(stderr, "%s: shm_close failed on semUsed!\n", programName);
                    }
                    if (sem_close(semWrite) == -1) {
                        fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
                    }
                    exit(EXIT_FAILURE);
                }
            }
            terminate = circularBuf->terminate;
            if (terminate != 0) {
                if (sem_post(semWrite) == -1) {
                    if (errno != EINTR) {
                        fprintf(stderr, "%s: sem_wait: failed!\n", programName);
                        free(withoutDup);
                        free(vertices);
                        free(random);
                        if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
                            fprintf(stderr, "%s: munmap failed!\n", programName);
                        }
                        if (close(shmfd) == -1) {
                            fprintf(stderr, "%s: close failed!\n", programName);
                        }
                        if (sem_close(semFree) == -1) {
                            fprintf(stderr, "%s: shm_close failed on semFree!\n", programName);
                        }
                        if (sem_close(semUsed) == -1) {
                            fprintf(stderr, "%s: shm_close failed on semUsed!\n", programName);
                        }
                        if (sem_close(semWrite) == -1) {
                            fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
                        }
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            }

            writeCB(*fbA, circularBuf, semFree, semUsed, programName);

            if (sem_post(semWrite) == -1) {
                if (errno != EINTR) {
                    fprintf(stderr, "%s: sem_wait: failed!\n", programName);
                    free(withoutDup);
                    free(vertices);
                    free(random);
                    if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
                        fprintf(stderr, "%s: munmap failed!\n", programName);
                    }
                    if (close(shmfd) == -1) {
                        fprintf(stderr, "%s: close failed!\n", programName);
                    }
                    if (sem_close(semFree) == -1) {
                        fprintf(stderr, "%s: shm_close failed on semFree!\n", programName);
                    }
                    if (sem_close(semUsed) == -1) {
                        fprintf(stderr, "%s: shm_close failed on semUsed!\n", programName);
                    }
                    if (sem_close(semWrite) == -1) {
                        fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
                    }
                    exit(EXIT_FAILURE);
                }
            }
        }
        // free memory of random vertice list in loop
        free(random);
    }

// ------------------------------- loop end --------------------------------------   
int error = 0;
// unmap and close shm
    if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
        fprintf(stderr, "%s: munmap failed!\n", programName);
        error = 1;
    }

    if (close(shmfd) == -1) {
        fprintf(stderr, "%s: close failed!\n", programName);
        error = 1;
    }

// close semaphores
    if (sem_close(semFree) == -1) {
        fprintf(stderr, "%s: shm_close failed on semFree!\n", programName);
        error = 1;
    }
    if (sem_close(semUsed) == -1) {
        fprintf(stderr, "%s: shm_close failed on semUsed!\n", programName);
        error = 1;
    }
    if (sem_close(semWrite) == -1) {
        fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
        error = 1;
    }

// free dynamically allocated memory
    free(withoutDup);
    free(vertices);

    if (error == 1) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}