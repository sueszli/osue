/**
 * @file generator.c
 * @author Stefan Hettinger <e11909446@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief generator program module.
 * 
 * @details This program implements part of the first assignment (B) called "fb_arc_set".
 * The generator takes 1 to n edges representing a graph in the form of u->v (e.g. 0-1 1-2 2-5),
 * then calculates a feedback arc set. If it is lower than the previous, it is written to the
 * circular buffer.
 **/

#include "supervisor.h"

//global variables
static const char *PROGNAME = "undefined";
static circular_buffer *circ_buffer = NULL; //the circular buffer variable
static sem_t *usedSem = NULL, *freeSem = NULL, *busySem = NULL; //the semaphore variables

//List of functions:
static int checkRegex(char *edge);
static void init_shm(int shm_fd);
static void cleanup(int shm_fd);
static void usage(char *message);
static void exitFailure(char *message);
static void exitFailureErrno(char *msg, char *errno_details);

/**
 * Program entry point.
 * @brief The main function parses the input edges and calculates feedback arc sets.
 * @details This function firstly compares the input edges against a simple regex pattern.
 * It then stores the edges in an array, finds the number of used vertices, calculates
 * a mask to randomize the vertices, then finds the feedback arc set and writes it to the
 * circualr buffer, if the set contains less edges than before. When writing to the buffer,
 * the best value that the supervisor has received is also fetched to reduce the writing
 * of non-optimal solutions.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int main(int argc, char *argv[]) {
    PROGNAME = argv[0];
    int shm_fd = -1;
    size_t nr_of_edges;
    size_t nr_of_vertices = 0;

    //handle input
    int opt;
    if((opt = getopt(argc, argv, "")) == '?') {
        usage("No argument expected! (opt = ?)");
    } else if(opt != -1) {
        exitFailure("No argument expexted! (opt != -1)");
    }

    if(argc == 1)
        usage("No edges found!");

    nr_of_edges = argc-1; //-1 because of program name
    edge edges[nr_of_edges]; //create array for edges

    int i;
    for(i = 0; i < argc-1; i++) { //loop over possible edges
                
        if(checkRegex(argv[i+1]) != 0) //check if edge is valid
            exitFailureErrno("Input is no valid edge!", argv[i+1]); //not really using errno but same format so it works

        char *tempEdge = strdup(argv[i+1]); 

        char *vertexOneAsString = strtok(tempEdge, "-"); //splits the edge by "-" into vertexOne and vertexTwo
        char *vertexTwoAsString = strtok(NULL, "-");

        long vertexOne = strtol(vertexOneAsString, NULL, 10);
        long vertexTwo = strtol(vertexTwoAsString, NULL, 10);

        free(tempEdge); //not needed anymore

        if (vertexOne == LONG_MIN || vertexTwo == LONG_MAX) //check for expected vertex range
            exitFailure("Vertex number too big!");

        if(vertexOne < 0 || vertexTwo < 0) //check for negative vertex
            exitFailure("Vertex should be positive!");

        edges[i].u = vertexOne;
        edges[i].v = vertexTwo;


        if(vertexOne > nr_of_vertices)
            nr_of_vertices = vertexOne;

        if(vertexTwo > nr_of_vertices)
            nr_of_vertices = vertexTwo;
    }
    nr_of_vertices++; //assumption: all graphs include vertex 0 to n (not all have to have edges)

    init_shm(shm_fd); //initialize shared memory

    //open sem
    busySem = sem_open(BUSY_SEM, 0);
    if(busySem == SEM_FAILED) {
        cleanup(shm_fd);
        exitFailureErrno("Could not open busySem!", strerror(errno));
    }
    freeSem = sem_open(FREE_SEM, 0);
    if (freeSem == SEM_FAILED) {
        cleanup(shm_fd);
        exitFailureErrno("Could not open freeSem!", strerror(errno));
    }
    usedSem = sem_open(USED_SEM, 0);
    if (usedSem == SEM_FAILED) {
        cleanup(shm_fd);
        exitFailureErrno("Could not open usedSem!", strerror(errno));
    }

    circ_buffer->nr_of_gen++; //generator is ready so it gets registered (necessary for termination)

    //generate random seed
    time_t t = (unsigned) time(NULL);
    srand(t * getpid());

    int actual_best = 8; //to store previous minimum

    while(circ_buffer->term == 0) { //run as long as no shutdown occures

        //generate random vertex mask
        double random_vertex_mask[nr_of_vertices];
        int i;
        for(i = 0; i < nr_of_vertices; i++) { //fill mask
            random_vertex_mask[i] = i;
        }

        int j, random_nr;
        for(j = 0; j < nr_of_vertices; j++) { //'dreieckstausch' with random number
            random_nr = rand() % (nr_of_vertices);
            double temp = random_vertex_mask[random_nr];
            random_vertex_mask[random_nr] = random_vertex_mask[j];
            random_vertex_mask[j] = temp;
        }

        int nr_of_edges_to_remove = 0;
        edge_array edges_to_remove;
        edges_to_remove.nr_of_edges = 0;
        int k;
        for(k = 0; k < nr_of_edges && nr_of_edges_to_remove < actual_best; k++) { //check for feedback loops

            if(random_vertex_mask[edges[k].u] > random_vertex_mask[edges[k].v]) {
                edges_to_remove.edges[nr_of_edges_to_remove] = edges[k];
                edges_to_remove.nr_of_edges = ++nr_of_edges_to_remove;
            }
        }

        if(nr_of_edges_to_remove >= actual_best) { //ignore if solution is worse
            continue;
        } else { //write to circular buffer

            //fprintf(stderr, "%d wrote %d\n", getpid(), nr_of_edges_to_remove); //can be enabled to better visualize

            //wait for buffer
            if(sem_wait(freeSem) < 0) {
                cleanup(shm_fd);
                if(errno == EINTR)
                    exit(EXIT_SUCCESS);

                exitFailureErrno("Could not wait on freeSem!", strerror(errno));
            }
            if(circ_buffer->term != 0) { //only write if no termination
                cleanup(shm_fd);
                exit(EXIT_SUCCESS);
            }
            
            if(sem_wait(busySem)) { //wait for mutex
                cleanup(shm_fd);
                if(errno == EINTR)
                    exit(EXIT_SUCCESS);

                exitFailureErrno("Could not wait on busySem!", strerror(errno));
            }

            //write to buffer
            circ_buffer->fb_arc_sets[circ_buffer->next_write] = edges_to_remove;
            circ_buffer->next_write = (circ_buffer->next_write +1) % BUFFER_SIZE;
            
            int buffer_best = circ_buffer->best; //fetch best solution from buffer for next time
            actual_best = nr_of_edges_to_remove < buffer_best ? nr_of_edges_to_remove : buffer_best;

            //indicate done
            if(sem_post(busySem) < 0) {
                cleanup(shm_fd);
                exitFailureErrno("Could not post busySem!", strerror(errno));
            }

            if(sem_post(usedSem) < 0) {
                cleanup(shm_fd);
                exitFailureErrno("Could not post usedSem!", strerror(errno));
            }
        }
    }
    cleanup(shm_fd);
    exit(EXIT_SUCCESS);
}

/**
 * @brief This function compares one of the provided edges to a regex string.
 * @details One edge can only contain "VERTEX1 [dash] VERTEX2". This function throws an error if a
 * "wrong" edge is detexted
 * @param edge One edge of the input.
 * @return Returns 0 on success and -1 otherwise.
 */
static int checkRegex(char *edge) {
    regex_t regex;
    int regexPattern = regcomp(&regex, "^[0-9]+-[0-9]+$", REG_EXTENDED); //only allows "NUMBER-NUMBER"

    regexPattern = regexec(&regex, edge, 0, NULL, 0); //compares edge with pattern
    regfree(&regex);
    return regexPattern == 0 ? 0 : -1; //0 if ok, -1 otherwise
}

/**
 * @brief The function init_shm initializes the shared memory.
 * @details This function opens and maps the shared memory file descriptor for the
 * circular buffer.
 * @param shm_fd the shared memory file descriptor.
 * @return Returns void, calls EXIT_FAILURE.
 */
static void init_shm(int shm_fd) {
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if(shm_fd < 0) {
        if(errno == ENOENT)
            exitFailure("Could not detect supervisor!");

        exitFailureErrno("Could not open shm_fd!", strerror(errno));
    }

    circ_buffer = mmap(NULL, sizeof(*circ_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(circ_buffer == MAP_FAILED)
        exitFailureErrno("Shared memory could not be mapped!", strerror(errno));

    if (close(shm_fd) < 0)
        exitFailureErrno("shm_fd could not be closed!", strerror(errno));
}

/**
 * @brief The function is used for cleanup.
 * @details This function is used to unmap the shared memory and to
 * close all the semaphores.
 * @param shm_fd the shared memory file descriptor.
 * @return Returns void, does cleanup of shm and sem.
 */
static void cleanup(int shm_fd) {
    if(circ_buffer != NULL) {
        circ_buffer->nr_of_gen--;
        if(munmap(circ_buffer, sizeof(*circ_buffer)) < 0)
            exitFailureErrno("shm could not be unmapped!", strerror(errno));
    }

    if(shm_fd != -1) { //close shm_fd and sem
        if(close(shm_fd) < 0)
            exitFailureErrno("shm_fd could not be closed!", strerror(errno));
    }
    if(usedSem != NULL) {
        if(sem_close(usedSem) < 0)
            exitFailureErrno("usedSem could not be closed!", strerror(errno));
    }
    if(freeSem != NULL) {
        if(sem_close(freeSem) < 0)
            exitFailureErrno("freeSem could not be closed!", strerror(errno));
    }
    if (busySem != NULL) {
        if (sem_post(busySem) < 0) //can clear DL
            exitFailureErrno("sem_post for busySem failed!", strerror(errno));

        if (sem_close(busySem) < 0)
            exitFailureErrno("busySem could not be closed!", strerror(errno));
    }
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
    fprintf(stderr, "Usage: %s EDGE1...\n", PROGNAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief The function exitFailure is used to exit the program and write a provided
 * error message to stderr.
 * @details This function is called whenever unexpected behavor is detected. The
 * error message has to be provided manually.
 * @param msg The provided error message.
 * @return Returns void, calls EXIT_FAILURE.
 */
static void exitFailure(char *msg) {
    fprintf(stderr, "%s ERROR: \"%s\"\n", PROGNAME, msg);
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


//long boii
//0-2 0-9 0-11 1-4 3-2 3-6 4-2 4-9 5-2 5-11 6-2 6-4 7-2 7-4 7-5 7-8 7-16 7-17 8-9 8-12 8-17 10-2 10-9 11-2 12-1 12-6 12-10 13-5 13-6 13-8 14-4 14-12 15-8 15-11 15-13 16-1 16-6 16-17 17-6 17-10 17-11 18-7 18-8 18-11
