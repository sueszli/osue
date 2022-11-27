/**
* @file generator.c
* @author Miriam Brojatsch 11913096
* @date 14.11.2021
*
* @brief This file implements the functionality of a generator that takes a graph and randomly generates a set of edges to be removed for it to be acyclic until the supervisor tells it to stop.
* If a solution is sufficiently small, it is written to a circular buffer for the supervisor to read. Repeat. Many of these generators can be run at once and the circular buffer should still work.
**/

#include "general.h"

//program name
static const char* NAME;

//how many vertices and edges are in the graph
int vertices_count = 0;
int edge_count = 0;


/**
* @brief Prints usage message and writes to stderr. Then exit on failure.
**/
static void USAGE(void) {
    fprintf(stderr, "Usage: %s EDGE1 EDGE2 ...\n", NAME);
    fprintf(stderr, "Example: %s 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0\n", NAME);
    exit(EXIT_FAILURE);
}



/**
* @brief Takes one argument that should look like "<somenumbergreaterorequalzero1>-<somenumbergreaterorequalzero2>", checks the format and numbers,
* then extracts the two numbers into a new struct edge {somenumbergreaterorequalzero1, somenumbergreaterorequalzero2}. Also updates the global count of vertices.
* @param arg: pointer to the start of one input argument of main
**/
static edge get_edge(char* arg) {
    char *arg_copy = strdup(arg); //better not mess with the real thing

    if (arg_copy == NULL) {
        printf("Something went wrong while parsing an edge");
        USAGE();
    }

    //get u (start node of edge)
    char * end;
    char * start = arg_copy;
    long u = strtol(start, &end, 0); //0 means compiler decides which base to use

    if (end == start) {
        fprintf(stderr, "Invalid vertex: %s\n", start);
        free(arg_copy);
        USAGE();
    }
    if (end[0] != '-') {
        fprintf(stderr, "Invalid delimiter: %s, please use '-'\n", end);
        free(arg_copy);
        USAGE();
    }
    if (u < 0) {
        fprintf(stderr, "Please use non-negative Index next time\n");
        free(arg_copy);
        USAGE();
    }



    //get v (end node of edge)
    start = end + 1; //next char after delimiter '-'
    long v = strtol(start, &end, 0); //0 means compiler decides which base to use

    if (end == start) {
        fprintf(stderr, "Invalid vertex: %s\n", start);
        free(arg_copy);
        USAGE();
    }
    if (end[0] != '\0') {
        fprintf(stderr, "Invalid delimiter: %s, please use '-'\n", end);
        free(arg_copy);
        USAGE();
    }
    if (u < 0) {
        fprintf(stderr, "Please use non-negative Index next time\n");
        free(arg_copy);
        USAGE();
    }


    free(arg_copy);
    int max = (u > v ? u : v);

    if (max + 1 > vertices_count) { //greatest index of a vertex is 1 smaller than vertex count
        vertices_count = max + 1;
    }

    //create a new edge with inputs u and v
    edge newedge = {(int) u, (int) v};
    return newedge;
}



/**
* @brief Takes argumentcounter of main and argv, then checks for arguments, calls get_edge on each argument and writes it to edges[].
* @param argc: argument counter of main
* @param argv: arguments of main
* @param edges: "output" of this function is stored here, should be empty before
**/
static void get_graph(int argc, char ** argv, edge edges[]) {
    if (argc == 1) { //no edges
        USAGE();
    }

    for (int i = 1; i < argc; i++) {
        edges[i-1] = get_edge(argv[i]);
    }
}


/**
* @brief Implements Fisher-Yates-Shuffle. Takes an int array and shuffles it in place randomly (as told by random() function).
* @param vertices: just ints that wanna be shuffled, not ordered oor anything
**/
static void fisher_yates_shuffle(int vertices[]) {
    for (int i = vertices_count-1; i >= 1; --i) {
		int rand = random() % (i + 1);
		int tmp = vertices[i];
		vertices[i] = vertices[rand];
		vertices[rand] = tmp;
    }
}


/**
* @brief Takes a solution set and writes it to the next free bin in the buffer.
* @details Synchronisation is realized using semaphores, before writing makes sure that please_exit flag is not set.
* First request semaphore, then wait for mutually exclusive access, write to buffer and finally post the semaphores.
* @param solution: solution set that wants to be printed to buffer
**/
static void write_buffer(solution_set solution) {
    
    if (sem_wait(sem_free) == -1) {
        if (errno == EINTR) {
            exit(EXIT_SUCCESS);
        }
        fprintf(stderr, "An error occurred while waiting to write in %s\n", NAME);
        exit(EXIT_FAILURE);
    }

    //check if maybe in the meantime supervisor has decided he wants generators to stop
    if (buffer->please_exit) {
        exit(EXIT_SUCCESS);
    }

    if (sem_wait(sem_mutex)) {
        if (errno == EINTR) {
            exit(EXIT_SUCCESS);
        }
        fprintf(stderr, "An error occurred while waiting to write in %s\n", NAME);
        exit(EXIT_FAILURE);
    }

    //now actually write
    buffer->data[buffer->write_at] = solution;
    buffer->write_at = (buffer->write_at + 1) % BUFFER_SIZE;

    
    if (sem_post(sem_mutex) == -1) {
        fprintf(stderr, "An error occurred while posting sem_mutex in %s", NAME);
        exit(EXIT_FAILURE);
    }
    if (sem_post(sem_used) == -1) {
        fprintf(stderr, "An error occurred while posting sem_used in %s", NAME);
        exit(EXIT_FAILURE);
    }
}


/**
* @brief Cleanup. Free resources.
**/
void clean_all(void){
  munmap(buffer, sizeof(*buffer));
  close(shm_fd);
  sem_close(sem_free);
  sem_close(sem_used);
  sem_close(sem_mutex);
}



/**
* @brief This is the main function. It sets up the infrastructure of the graph by reading in argv, then opens shared memory and semaphores, and repeatedly generates solutions to write to buffer.
* @param argc: argument counter
* @param argv: input arguments
**/
int main(int argc, char **argv) {
    NAME = argv[0];

    //take set of edges as input (however long)
    edge_count = (argc-1);
    edge graph[edge_count];
    get_graph(argc, argv, graph); //also handles case of no arguments


    int shuffled_vertices[vertices_count]; //not shuffled yet
    for (int i = 0; i < vertices_count; ++i) { //initialize with all values 0 to vertices_count - 1
        shuffled_vertices[i] = i;
    }


    //open shared memory
    shm_fd = shm_open(MATRICULATION_NO, O_RDWR, 0600);
    if (shm_fd == -1) { //because either shm_open failed or was not called
        fprintf(stderr, "Shared memory could not be opened in %s\n", NAME);
        exit(EXIT_FAILURE);
    }

    //map memory
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buffer == MAP_FAILED) {
        fprintf(stderr, "Mapping could not be created in %s\n", NAME);
        close(shm_fd);
        shm_unlink(MATRICULATION_NO);
        exit(EXIT_FAILURE);
    }

    sem_free = sem_open(SEM_FREE_NAME, 0);
    sem_used = sem_open(SEM_USED_NAME, 0);
    sem_mutex = sem_open(SEM_MUTEX_NAME, 0);


    //repeatedly generate random solution if there is no order to exit
    while (buffer->please_exit == 0) {
        //generate random vertex array
        fisher_yates_shuffle(shuffled_vertices); //any permutation is as likely as any other, so no need to reorder the elements to (0, 1, 2,..)


        solution_set possible_solution;
        for (int i = 0; i < 8; ++i) { //initialize with all values 0 to avoid segfault maybe
            possible_solution.edges[i].u = 0;
            possible_solution.edges[i].v = 0;
        }
        possible_solution.size = 0;

        //chooses edges according to algorithm
        for (int i = 0; i < edge_count && possible_solution.size < 7; ++i) {
            int u_position = shuffled_vertices[graph[i].u];
            int v_position = shuffled_vertices[graph[i].v];
            if (u_position > v_position) {
                possible_solution.edges[possible_solution.size] = graph[i];
                possible_solution.size++;
            }
        }

        //check if solution is good enough
        if (possible_solution.size == 7) {
            continue;
        } else {
            //write result to buffer
            write_buffer(possible_solution);
        }

    }

    //cleanup
    clean_all();
    return 0;

}
