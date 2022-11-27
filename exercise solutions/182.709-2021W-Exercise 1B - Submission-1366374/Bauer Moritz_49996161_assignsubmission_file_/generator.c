/**
 * @brief generator generates new solutions for Feedback Arc Set
 * @author Moritz Bauer, 0649647
 * @date 21.11.11
 */
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <getopt.h>    /* for getopt */
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "shuffle.h"
#include "feedback_arc_set.h"


#ifdef DEBUG
#define debug(out,msg) \
    (void) fprintf(out, "[%s:%d] " msg, __FILE__, __LINE__);
#define fdebug(out,fmt, ...) \
    (void) fprintf(out, "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define debug(...)
#define fdebug(...)
#endif

#define MAX(a, b) (((a)>(b))?(a):(b))

/**
 * @brief munmap shared_memory
 * @param shared_memory pointer for shared_memory
 * @param argv_0 executed filename
 */
static void cleanup_shm(struct shared_memory *shared_memory, char *argv_0) {
    // unmap shared memory:
    if (munmap(shared_memory, sizeof(*shared_memory)) == -1) {
        fprintf(stderr, "[%s] munmap failed.\n", argv_0);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief open and map shared_memory
 * @param argv_0
 * @return returns a pointer to shared mem
 */
static struct shared_memory *setup_shm(char *argv_0) {
    struct shared_memory *shared_memory;
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
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
    return shared_memory;
}

/**
 * @brief setup a vertex array
 * @param vertices a pointer to a array of length size
 * @param size number of vertices
 * @return pointer to a array
 */
static void init_vertices(long *vertices, int size) {
    for (int i = 0; i < size; i++) {
        vertices[i] = i;
    }
}

/**
 * @brief pare a long
 * @param str
 * @return return the number
 */
static long parseLong(char *str, char *argv_0) {
    char *ptr;
    long val = strtol(str, &ptr, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        fprintf(stderr, "[%s] strtol failed: %s\n", argv_0, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (ptr == str) {
        fprintf(stderr, "[%s] No digits were found.\n", argv_0);
        exit(EXIT_FAILURE);
    }
    return val;
}


int main(int argc, char *argv[]) {


    // create semaphores
    sem_t *sem_free = sem_open(SEM_FREE, 0);
    if (sem_free == NULL) {
        fprintf(stderr, "[%s] could not sem_open (free): %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    sem_t *sem_used = sem_open(SEM_USED, 0);
    if (sem_used == NULL) {
        fprintf(stderr, "[%s] could not sem_open (used): %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);
    if (sem_mutex == NULL) {
        fprintf(stderr, "[%s] could not sem_open (mutex): %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }


    // setup shared mem
    struct shared_memory *shared_memory = setup_shm(argv[0]);
    {

        // use pid to get random seeds - time(0) is not enough
        int seed = time(0) + getpid();
        srandom(seed);

        int c;
        while ((c = getopt(argc, argv, "")) != -1) {
            //asserts no parameters
            switch (c) {
                default:
                    fprintf(stderr, "Usage: %s [Edges...]\n",
                            argv[0]);
                    exit(EXIT_FAILURE);
            }
        }
        int num_edges = argc - optind;

        // alloc edges, this is possible because we know that the number of edges (except for invalid edges)
        struct Edge *edges = malloc(sizeof(struct Edge) * num_edges);
        {
            long max_vertex = -1;

            // read edges from argv
            for (int i = optind; i < argc; ++i) {

                char *token_from = strtok(argv[i], "-");
                char *token_to = strtok(NULL, "-");
                char *token_empty = strtok(NULL, "");
                if (token_empty != NULL) {
                    // inputs like `./generator 1-0 1-3-1` are considered invalid ?!
                    fprintf(stderr, "[%s] Invalid input.\n", argv[0]);
                    exit(EXIT_FAILURE);
                }

                long from = parseLong(token_from, argv[0]);
                long to = parseLong(token_to, argv[0]);
                edges[i - optind].from = from;
                edges[i - optind].to = to;
                max_vertex = MAX(MAX(max_vertex, from), to);
            }

            int size = max_vertex + 1; // +1 because first edge is indexed 0

            long *vertices = malloc(sizeof(long) * size);
            init_vertices(vertices, size);

            // reusable Feedback_Arc_Set on stack
            struct Feedback_Arc_Set feedback_arc_set;

            // while !quit
            // shuffle
            // do the topological sort thing
            // memcpy into shared memory

            while (1) {
                if (shared_memory->quit) {
                    break;
                }


                shuffle_array(vertices, size);

                feedback_arc_set.size = 0;
                for (int j = 0; j < num_edges; ++j) {
                    struct Edge edge = edges[j];
                    if (vertices[edge.from] < vertices[edge.to]) {

                        feedback_arc_set.edges[feedback_arc_set.size] = edge;
                        feedback_arc_set.size++;
                    }

                }

                struct Feedback_Arc_Set *ptr = &(shared_memory->buffer.buffer[shared_memory->buffer.front]);

                sem_wait(sem_free);
                if (shared_memory->quit) {
                    sem_post(sem_free);
                    break;
                }

                sem_wait(sem_mutex);
                {
                    memcpy(ptr, &feedback_arc_set, sizeof(struct Feedback_Arc_Set));
                    shared_memory->buffer.front = (shared_memory->buffer.front + 1) % BUFFER_SIZE;
                    fdebug(stdout, "ptr:%d\n", ptr->size);
                }
                sem_post(sem_mutex);

                sem_post(sem_used);

                if (feedback_arc_set.size == 0) {
                    // break here because supervisor will EVENTUALLY set shared_memory->quit
                    // and will not accept any other results from THIS generator
                    break;
                }
            }

// cleanup everything

            free(vertices);
        }
        free(edges);
    }

    cleanup_shm(shared_memory, argv[0]);

    sem_close(sem_mutex);
    sem_close(sem_free);
    sem_close(sem_used);


    exit(EXIT_SUCCESS);
}
