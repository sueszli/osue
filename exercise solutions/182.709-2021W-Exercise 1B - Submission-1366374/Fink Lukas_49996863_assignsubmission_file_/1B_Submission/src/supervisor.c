/**
 * @file supervisor.c
 * @author Lukas Fink 11911069
 * @date 10.11.2021
 *
 * @brief supervisor module.
 * 
 * @details recieves data (Feedback-Arcs) from the generators.
 * it stores the best result received
 * every time a better result is received, it prints it to stdout.
 * when a result with 0 edges is received or the program is interrupted
 * with SIGINT or SIGTERM, it sets a termination value in the buffer to
 * properly terminate the generators.
 * After doing that, it closes all recources and terminates successfully.
 * Exists with EXIT_FAILURE on failure
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
#include <signal.h>
#include <semaphore.h>
#include "datatypes.h"
#include "algorithms.h"
#include "circularBuffer.h"

static int deepCopyArc(struct FeedbackArc *dest, struct FeedbackArc *src);


volatile sig_atomic_t quit = 0; /** global variable to terminate the program **/

/**
 * Signal-Handler
 * @brief a signal handler which sets the global variable quit to 1
 * @details global variables: quit
 */
void handle_signal(int signal) { quit = 1; }

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters, and checks
 * if the input has the correct form.
 * It reads from the circular Buffer and prints the best solution.
 * @details It creates and unlinks all semaphores and the shared memory used by the
 * supervisor and the generators. When finding a solution with 0 edges or receiving
 * a SIGINT or SIGTERM the program sets a terminate-value in the circular Buffer and
 * then - after closing all recources - properly terminates.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS when succeeding, otherwise EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    char *programName = argv[0];
    if (argc > 1) {
        fprintf(stderr, "%s: arguments not allowed!\n", programName);
        exit(EXIT_FAILURE);
    }

    

// open, truncate and map shared memory
    int shmfd = shm_open(SHM_CIRCULAR_BUFFER, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "%s: shm_open failed!\n", programName);
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmfd, sizeof(struct CircularBuffer)) < 0) {
        fprintf(stderr, "%s: ftruncate failed!\n", programName);
        if (close(shmfd) == -1) {
            fprintf(stderr, "%s: close failed on shmfd!\n", programName);
        }
        if (shm_unlink(SHM_CIRCULAR_BUFFER) == -1) {
            fprintf(stderr, "%s: shm_unlink failed on SHM_CIRCULAR_BUFFER!\n", programName);
        }
        exit(EXIT_FAILURE);
    }

    struct CircularBuffer *circularBuf;
    circularBuf = mmap(NULL, sizeof(*circularBuf), PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (circularBuf == MAP_FAILED) {
        fprintf(stderr, "%s: mmap failed!\n %s", programName,strerror(errno));
        if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
            fprintf(stderr, "%s: munmap failed!\n", programName);
        }
        if (close(shmfd) == -1) {
            fprintf(stderr, "%s: close failed on shmfd!\n", programName);
        }
        if (shm_unlink(SHM_CIRCULAR_BUFFER) == -1) {
            fprintf(stderr, "%s: shm_unlink failed on SHM_CIRCULAR_BUFFER!\n", programName);
        }
        exit(EXIT_FAILURE);
    }
// initialise read/write Positions and the terminate value
    circularBuf->readPos = 0;
    circularBuf->writePos = 0;
    circularBuf->terminate = 0;

// create Semaphores for synchronisation with generators(semFree/Used) and for sync. between generators (semWrite)
    sem_t *semFree = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_DATA);    // MAX_DATA = 32, at begin the whole space is free
    sem_t *semUsed = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);           // no space is used at the begin
    sem_t *semWrite = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);

    if(semFree == SEM_FAILED || semUsed == SEM_FAILED || semWrite == SEM_FAILED){
        fprintf(stderr, "%s: sem_open failed\n", programName);
        if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
            fprintf(stderr, "%s: munmap failed!\n", programName);
        }
        if (close(shmfd) == -1) {
            fprintf(stderr, "%s: close failed on shmfd!\n", programName);
        }
        if (shm_unlink(SHM_CIRCULAR_BUFFER) == -1) {
            fprintf(stderr, "%s: shm_unlink failed on SHM_CIRCULAR_BUFFER!\n", programName);
        }
        if (semFree == SEM_FAILED) {
            if (sem_close(semFree) == -1) {
                fprintf(stderr, "%s: sem_close failed on semFree!\n", programName);
            }
            if (sem_unlink(SEM_FREE) == -1) {
                fprintf(stderr, "%s: sem_unlink failed on SEM_FREE!\n", programName);
            }
        }
        if (semUsed == SEM_FAILED) {
            if (sem_close(semUsed) == -1) {
                fprintf(stderr, "%s: sem_close failed on semUsed!\n", programName);
            }
            if (sem_unlink(SEM_USED) == -1) {
                fprintf(stderr, "%s: sem_unlink failed on SEM_USED!\n", programName);
            }
        }
        if (semWrite == SEM_FAILED) {
            if (sem_close(semWrite) == -1) {
                fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
            }
            if (sem_unlink(SEM_WRITE) == -1) {
                fprintf(stderr, "%s: shm_unlink failed on SEM_WRITE!\n", programName);
            }
        }
        exit(EXIT_FAILURE);
    }
// allocate memory for best Arc
    struct FeedbackArc *bestArc = malloc(sizeof(struct FeedbackArc));
    if (bestArc == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
            fprintf(stderr, "%s: munmap failed!\n", programName);
        }
        if (close(shmfd) == -1) {
            fprintf(stderr, "%s: close failed on shmfd!\n", programName);
        }
        if (shm_unlink(SHM_CIRCULAR_BUFFER) == -1) {
            fprintf(stderr, "%s: shm_unlink failed on SHM_CIRCULAR_BUFFER!\n", programName);
        }
        if (sem_close(semFree) == -1) {
            fprintf(stderr, "%s: sem_close failed on semFree!\n", programName);
        }
        if (sem_close(semUsed) == -1) {
            fprintf(stderr, "%s: sem_close failed on semUsed!\n", programName);
        }
        if (sem_close(semWrite) == -1) {
            fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
        }
        if (sem_unlink(SEM_FREE) == -1) {
            fprintf(stderr, "%s: sem_unlink failed on SEM_FREE!\n", programName);
        }
        if (sem_unlink(SEM_USED) == -1) {
            fprintf(stderr, "%s: sem_unlink failed on SEM_USED!\n", programName);
        }
        if (sem_unlink(SEM_WRITE) == -1) {
            fprintf(stderr, "%s: shm_unlink failed on SEM_WRITE!\n", programName);
        }
        exit(EXIT_FAILURE);
    }
    readCB(bestArc, circularBuf, semFree, semUsed, programName);
    int bestSize = getSizeEdges(bestArc->feedback);
    
    if (quit != 1) {
        fprintf(stdout, "[%s] Solution with %d edges: ", programName, bestSize);
        showEdges(bestArc->feedback);
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    

// allocate memory for reading in Arcs
    struct FeedbackArc *readArc = malloc(sizeof(struct FeedbackArc));
    if (readArc == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        free(bestArc);
        if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
            fprintf(stderr, "%s: munmap failed!\n", programName);
        }
        if (close(shmfd) == -1) {
            fprintf(stderr, "%s: close failed on shmfd!\n", programName);
        }
        if (shm_unlink(SHM_CIRCULAR_BUFFER) == -1) {
            fprintf(stderr, "%s: shm_unlink failed on SHM_CIRCULAR_BUFFER!\n", programName);
        }
        if (sem_close(semFree) == -1) {
            fprintf(stderr, "%s: sem_close failed on semFree!\n", programName);
        }
        if (sem_close(semUsed) == -1) {
            fprintf(stderr, "%s: sem_close failed on semUsed!\n", programName);
        }
        if (sem_close(semWrite) == -1) {
            fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
        }
        if (sem_unlink(SEM_FREE) == -1) {
            fprintf(stderr, "%s: sem_unlink failed on SEM_FREE!\n", programName);
        }
        if (sem_unlink(SEM_USED) == -1) {
            fprintf(stderr, "%s: sem_unlink failed on SEM_USED!\n", programName);
        }
        if (sem_unlink(SEM_WRITE) == -1) {
            fprintf(stderr, "%s: shm_unlink failed on SEM_WRITE!\n", programName);
        }
        exit(EXIT_FAILURE);
    }

// ---------------------------------------- loop begin -------------------------------------------
    while ((bestSize != 0) && (quit != 1) ) {
        readCB(readArc, circularBuf, semFree, semUsed, programName);
        int readSize = getSizeEdges(readArc->feedback);

        if (readSize < bestSize) {
            int succ = deepCopyArc(bestArc, readArc);
            if (succ == -1) {
                fprintf(stderr, "%s: deepCopyArc: failed!\n", programName);
                free(bestArc);
                free(readArc);
                if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
                    fprintf(stderr, "%s: munmap failed!\n", programName);
                }
                if (close(shmfd) == -1) {
                    fprintf(stderr, "%s: close failed on shmfd!\n", programName);
                }
                if (shm_unlink(SHM_CIRCULAR_BUFFER) == -1) {
                    fprintf(stderr, "%s: shm_unlink failed on SHM_CIRCULAR_BUFFER!\n", programName);
                }
                if (sem_close(semFree) == -1) {
                    fprintf(stderr, "%s: sem_close failed on semFree!\n", programName);
                }
                if (sem_close(semUsed) == -1) {
                    fprintf(stderr, "%s: sem_close failed on semUsed!\n", programName);
                }
                if (sem_close(semWrite) == -1) {
                   fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
                }
                if (sem_unlink(SEM_FREE) == -1) {
                    fprintf(stderr, "%s: sem_unlink failed on SEM_FREE!\n", programName);
                }
                if (sem_unlink(SEM_USED) == -1) {
                    fprintf(stderr, "%s: sem_unlink failed on SEM_USED!\n", programName);
                }
                if (sem_unlink(SEM_WRITE) == -1) {
                   fprintf(stderr, "%s: shm_unlink failed on SEM_WRITE!\n", programName);
                }
                exit(EXIT_FAILURE);
            }
            bestSize = readSize;
            if (bestSize == 0) {
                fprintf(stdout, "The graph is acyclic!\n");
                fflush(stdout);
            } else {
                fprintf(stdout, "[%s] Solution with %d edges: ", programName, bestSize);
                showEdges(bestArc->feedback);
                fprintf(stdout, "\n");
                fflush(stdout);
            }
        }      
    }
// ---------------------------------------- loop end ---------------------------------------------
int error = 0;
// terminate generators (write termination value in circular buffer)
    circularBuf->terminate = -1;
    if (sem_post(semFree) == -1) {              // increase semFree to allow generators to properly terminate
        if (errno != EINTR) {
            error = 1;
        }
    }

// unmap, close and unlink shm
    if (munmap(circularBuf, sizeof(*circularBuf)) == -1) {
        fprintf(stderr, "%s: munmap failed!\n", programName);
        error = 1;
    }
    if (close(shmfd) == -1) {
        fprintf(stderr, "%s: close failed on shmfd!\n", programName);
        error = 1;
    }
    if (shm_unlink(SHM_CIRCULAR_BUFFER) == -1) {
        fprintf(stderr, "%s: shm_unlink failed on SHM_CIRCULAR_BUFFER!\n", programName);
        error = 1;
    }
// close semaphores
    if (sem_close(semFree) == -1) {
        fprintf(stderr, "%s: sem_close failed on semFree!\n", programName);
        error = 1;
    }
    if (sem_close(semUsed) == -1) {
        fprintf(stderr, "%s: sem_close failed on semUsed!\n", programName);
        error = 1;
    }
    if (sem_close(semWrite) == -1) {
        fprintf(stderr, "%s: shm_close failed on semWrite!\n", programName);
        error = 1;
    }
// unlink semaphores
    if (sem_unlink(SEM_FREE) == -1) {
        fprintf(stderr, "%s: sem_unlink failed on SEM_FREE!\n", programName);
        error = 1;
    }
    if (sem_unlink(SEM_USED) == -1) {
        fprintf(stderr, "%s: sem_unlink failed on SEM_USED!\n", programName);
        error = 1;
    }
    if (sem_unlink(SEM_WRITE) == -1) {
        fprintf(stderr, "%s: shm_unlink failed on SEM_WRITE!\n", programName);
        error = 1;
    }


// free memory
    free(readArc);
    free(bestArc);

    if (error == 1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * deep copy function
 * @brief make a deep copy of an Feedback-Arc
 * @details copys values of src to dest and terminates
 * the edge array with an edge with negative values.
 * @param dest destination for the data to be copied
 * @param src source of data to be copied from
 * @return 0 on success, -1 on failure
 **/
static int deepCopyArc(struct FeedbackArc *dest, struct FeedbackArc *src) {
    if ((dest == NULL) || (src == NULL)) {
        return -1;
    }
    int n = 0;
    while (n <= MAX_EDGES) {
        dest->feedback[n].start = src->feedback[n].start;
        dest->feedback[n].end = src->feedback[n].end;
        n++;
    }
    dest->feedback[MAX_EDGES].start = -1;
    dest->feedback[MAX_EDGES].end = -1;
    return 0;
}