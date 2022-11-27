/**
 * @brief supervisor reads new solutions for Feedback Arc Set
 * @author Moritz Bauer, 0649647
 * @date 21.11.11
 */

#include <stdio.h>
#include <stdlib.h>    /* for exit */
#include <getopt.h>    /* for getopt */
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

#include "feedback_arc_set.h"


#ifdef DEBUG
#define debug(out,msg) \
    (void) fprintf(out, "[%s:%d] " msg  "\n", __FILE__, __LINE__);
#define fdebug(out,fmt, ...) \
    (void) fprintf(out, "[%s:%d] " fmt  "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define debug(...)
#define fdebug(...)
#endif

static volatile sig_atomic_t quit = 0;

/**
 * @brief signal handler for graceful shutdown
 * @param signal
 */
static void handle_signal(int signal) {
    quit = 1;
}

/**
 * @brief unmap and unlink shared mem
 * @param shared_memory
 * @param argv_0 executed file
 */
static void cleanup_shm(struct shared_memory *shared_memory, char *argv_0) {
    // unmap shared memory:
    if (munmap(shared_memory, sizeof(*shared_memory)) == -1) {
        fprintf(stderr, "[%s] munmap failed.\n", argv_0);
        exit(EXIT_FAILURE);
    }
    if (shm_unlink(SHM_NAME) == -1) {
        fprintf(stderr, "[%s] shm_unlink failed.\n", argv_0);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief create and map shared mem
 * @param argv_0 executed file
 * @return
 */
static struct shared_memory *setup_shm(char *argv_0) {
    // create the shared memory:
    struct shared_memory *shared_memory;
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "[%s] could not open shm: %s\n", argv_0, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct shared_memory)) < 0) {
        fprintf(stderr, "[%s] ftruncate failed.\n", argv_0);
        exit(EXIT_FAILURE);
    }

    // map shared memory object:
    shared_memory = mmap(NULL, sizeof(*shared_memory), PROT_READ | PROT_WRITE,
                 MAP_SHARED, shmfd, 0);
    if (shared_memory == MAP_FAILED) {
        fprintf(stderr, "[%s] mmap failed.\n", argv_0);
        exit(EXIT_FAILURE);
    }
    if (close(shmfd) == -1) {
        fprintf(stderr, "[%s] close shmfd failed: %s\n", argv_0, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // should be zero, but just to be sure
    shared_memory->quit = 0;
    return shared_memory;
}

int main(int argc, char *argv[]) {

    // set signal actions
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // setup shared mem
    struct shared_memory *shared_memory = setup_shm(argv[0]);
    {

        // create semaphores
        sem_t *sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
        if (sem_free == NULL) {
            fprintf(stderr, "[%s] could not sem_open (free): %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        sem_t *sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
        if (sem_used == NULL) {
            fprintf(stderr, "[%s] could not sem_open (used): %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        sem_t *sem_mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1);
        if (sem_mutex == NULL) {
            fprintf(stderr, "[%s] could not sem_open (mutex): %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }


        {
            // init min to some value grater than MAX_ARC_SET_SIZE
            int min = MAX_ARC_SET_SIZE + 1;

            while (!quit) {

                // wait until there is a new value in the ringbuffer
                sem_wait(sem_used);
                if (errno == EINTR) {
                    // handle interrupts and restart loop
                    sem_post(sem_free);
                    continue;
                }


                struct Feedback_Arc_Set *set = &shared_memory->buffer.buffer[shared_memory->buffer.end];

                shared_memory->buffer.end = (shared_memory->buffer.end + 1) % BUFFER_SIZE;

                if(set->size == 0){
                    // print message and stop execution
                    printf("[%s] The graph is acyclic\n", argv[0]);
                    sem_post(sem_free);
                    break;
                }
                if(set->size < min ){
                    // print message and set new min
                    min = set->size;
                    printf("[%s] Solution with %d edges:", argv[0], set->size);
                    for (int j = 0; j < set->size; ++j) {
                        printf(" %ld-%ld", set->edges[j].from, set->edges[j].to);
                    }
                    printf("\n");
                }

                sem_post(sem_free);
            }

            shared_memory->quit = 1;
        }

// close & unlink sem
        sem_close(sem_used);
        sem_close(sem_free);
        sem_close(sem_mutex);

        sem_unlink(SEM_USED);
        sem_unlink(SEM_FREE);
        sem_unlink(SEM_MUTEX);


    }
// unmap & unlink shm
    cleanup_shm(shared_memory, argv[0]);


    debug(stdout, "STOP");
    exit(EXIT_SUCCESS);
}
