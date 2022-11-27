/**
 * @file generator.c
 * @author Philipp Holzer <e12028208@student.tuwien.ac.at>
 * @date 2021-11-14
 * @brief generator
 * @details takes a graph as input, repeatedly generates a random solution and 
 * writes it's result to the circular buffer. Can shutdown, if supervisor notifies it. 
 */

#include "fb_arc_set.h"

static void usage(void);
static void emsg(char* msg, char* details);
static void eexit(char* msg, char* details);

int main(int argc, char *argv[]);

static edge pedge(const char *arg);
static void wwrite(edgec ec);

static void solutions(edge edges[]);
static void init(void);
static void shutdown(void);

static char* name;

static int shm_fd = -1; // shared memory circular buffer file descriptor

static circbuffer *circbuf = NULL; // circular buffer used to write and read solutions

static sem_t *sused = NULL; // space used in the shared buffer, used for signaling the supervisor that it can read data
static sem_t *sfree = NULL; // free space in shared buffer, used for signaling the generator that it can write data
static sem_t *smutex = NULL; // used by generators to ensure mutual exclusion while writing

static int ecount; // edges in input graph
static int vcount; // vertices of input graph

/**
 * @brief provide correct usage of programm written to stderr and exit program with a EXIT_FAILURE
 * @details global variable: name
 **/
static void usage(void) {
    fprintf(stderr, "USAGE: %s EDGE1 EDGE2 ...\nExample: %s 0-1 1-2 2-0\n", name, name);
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
 * @brief generates solutions and writes them to shared memory buffer
 * @details global variable: name
 */
int main(int argc, char* argv[]) {
    if(argc == 1){
        usage();
    }
    name = argv[0];

    ecount = argc - 1;
    edge edges[ecount];
    init();

    for (int i = 0; i < argc - 1; i++){
        edges[i] = pedge(argv[i + 1]); // parse input Parses a graph from the input args and fills edges array with it.
    }
    
    srand(time(NULL) * getpid()); // random seed for usage in rand 
                                  // (without srand program would create the same sequence of numbers each time it runs)
    solutions(edges);
    return EXIT_SUCCESS;
}

/**
 * @brief parses edge
 * @param arg parse edge from
 * @return parsed edge
 * @details global variable: circbuf
 */
static edge pedge(const char* arg) {
    char *input = strdup(arg), // duplicates string
         *ptr,
         *v1 = input;

    long f = strtol(v1, &ptr, 0); // error handling for first vertex
    if (ptr == v1) {
        fprintf(stderr, "ERROR in [%s]: invalid vertex index (\"%s\" is NaN)\n", name, v1); usage();
    }

    if (f < 0) {
        fprintf(stderr, "ERROR in [%s]: negative vertex \"%ld\" is not allowed\n", name, f); usage();
    }

    if (ptr[0] != '-') {
        fprintf(stderr, "ERROR in [%s]: invalid vertex delimiter \"%c\" (has to be \"-\")\n", name, ptr[0]); usage();
    }

    // parse second vertex

    char *v2 = ptr + 1;
    long t = strtol(v2, &ptr, 0); // error handling for second vertex
    if (ptr == v2) {
        fprintf(stderr, "ERROR in [%s]: invalid vertex index (\"%s\" is NaN)\n", name, v2); usage();
    }

    if (t < 0) {
        fprintf(stderr, "ERROR in [%s]: negative vertex %ld is not allowed\n", name, t); usage();
    }

    if (ptr[0] != '\0') {
        fprintf(stderr, "ERROR in [%s]: invalid edge delimiter \"%c\" (has to be \" \")\n", name, ptr[0]); usage();
    }

    // update number of vertices

    if (f + 1 > vcount) {
        vcount = f + 1;
    }
    if (t + 1 > vcount) {
        vcount = t + 1;
    }

    free(input); // release space

    return (edge) {.f = f, .t = t};
}

/**
 * @brief writes to the circular buffer (blocks if it can't write)
 * @param edgec containing a solution edge candidate (ec)
 * @details global variables: sused, sfree, smutex, circbuf
 */
static void wwrite(edgec ec) {
    // blocks until buffer can be written and mutual exclusion is guaranteed

    if (sem_wait(sfree) == -1) {
        if (errno == EINTR) {
            exit(EXIT_SUCCESS);
        }
        eexit("while sem_wait for \"sfree\"", strerror(errno));
    }
    if (circbuf -> shutdown) {
        exit(EXIT_SUCCESS);
    }
    if (sem_wait(smutex)) {
        if (errno == EINTR) {
            exit(EXIT_SUCCESS);
        }
        eexit("while sem_wait for \"smutex\"", strerror(errno));
    }

    circbuf -> data[circbuf -> write] = ec;
    circbuf -> write = (circbuf -> write + 1) % BSIZE;

    if(sem_post(smutex) == -1) { 
        eexit("can't unlock semaphore \"smutex\"", strerror(errno));
    }

    if (sem_post(sused) == -1) { // new data can be read
        eexit("can't unlock semaphore \"sused\"", strerror(errno));
    }
}


/**
 * @brief generates solutions and writes them to shared memory buffer
 * @param edges to generate solutions for
 * @details global variable: circbuf
 */
static void solutions(edge edges[]) {
    while (circbuf -> shutdown == 0) {
        int randomp[ecount]; // random permutation
        
        for (int i = 0; i < vcount; i++) { // fills array with the numbers from 0 to n - 1 (ascending)
            randomp[i] = i;
        }

        // generates a random permutation of given vertices by using the Fisher-Yates
        // shuffle algorithm (see: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle)
        for (int i = vcount - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int temp = randomp[j];
            randomp[j] = randomp[i];
            randomp[i] = temp;
        }
        
        edgec edelete; // edges to delete

        edelete.counter = 0; 
        int edcounter = 0;
        for (int i = 0; i < ecount && edcounter < 7; i++) {
            if(randomp[edges[i].t] <= randomp[edges[i].f]) {
                edelete.container[edcounter] = edges[i];
                edelete.counter = ++edcounter;
            }
        }

        if (edcounter < 7) {
            wwrite(edelete);
        }
    }
}

/**
 * @brief opens shared memory and opens semaphores
 * @details global variables: shm_fd, circbuf, sused, sfree, smutex
 */
static void init(void) {
    shm_fd = shm_open(SHMNAME, O_RDWR, 0600); // creates and opens new (or existing) shared memory object
    // O_RDWR = open the object for read-write access

    if (shm_fd == -1) {
        if (errno == ENOENT) {
            emsg("supervisor has to be started first", NULL);
        }
        eexit("unable to create shared memory", strerror(errno));
    }

    // map shared memory
    circbuf = mmap(NULL, sizeof(*circbuf), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    if (circbuf == MAP_FAILED) {
        eexit("couldn't map shared memory", strerror(errno));
    }

    if (close(shm_fd) == -1) {
        eexit("couldn't close shared memory \"shm_fd\"", strerror(errno));
    }

    // create semaphores or throw errors if unsuccessfull
    sused = sem_open(SUSED, 0);
    if (sused == SEM_FAILED) {
        eexit("could't create semaphore \"sused\"", strerror(errno));
    }

    sfree = sem_open(SFREE, 0);
    if (sfree == SEM_FAILED) {
        eexit("could't create semaphore \"sfree\"", strerror(errno));
    }

    smutex = sem_open(SMUTEX, 0);
    if (smutex == SEM_FAILED) {
        eexit("could't create semaphore \"smutex\"", strerror(errno));
    }

    shm_fd = -1;
    circbuf -> generatorcnt++;

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
        circbuf -> generatorcnt--;
        if (munmap(circbuf, sizeof(*circbuf)) == -1) {
            emsg("error unmapping shared memory", strerror(errno));
        }
    }

    if (shm_fd != -1) {
        if (close(shm_fd) == -1) { // close shared memory fd (if not already closed)
            emsg("couldn't close shared memory \"shm_fd\"", strerror(errno));
        }
    }

    if (sused != NULL) {
        if (sem_close(sused) == -1) { // close a named semaphore (sused)
            emsg("couldn't close semaphore \"sused\"", strerror(errno));
        }
    }

    if (sfree != NULL) {
        if (sem_close(sfree) == -1) { // close a named semaphore (sfree)
            emsg("couldn't close semaphore \"sfree\"", strerror(errno));
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
}