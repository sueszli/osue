/**
 * @file minimalSet3Coloring_generator.c
 * @author Aiden Foster 11910604
 * @date 12.11.2021
 *
 * @brief Generate random but valid solutions for minimal set 3 coloring, the graph is given via parameters
 * @details quit is a flag to be set to 1 by signal handler when the program should terminate
**/

#include "minimalSet3Coloring_generator.h"

/**
 * @brief Quit flag, this is set to 1 when the program should terminate
**/
volatile sig_atomic_t quit = 0;

/**
 * @brief Name of this program, equals argv[0]
**/
char* programname;

/**
 * usage function
 * @brief output a usage message to stderr and terminate
**/
static void usage(char* programname){
	fprintf(stderr, "Usage: %s Edges...\n\tExample: %s 0%c1 1%c2 1%c3 1%c4 2%c4 3%c6 4%c3 4%c5 6%c0\n", programname, programname, EDGE_SEPARATOR, EDGE_SEPARATOR, EDGE_SEPARATOR, EDGE_SEPARATOR, EDGE_SEPARATOR, EDGE_SEPARATOR, EDGE_SEPARATOR, EDGE_SEPARATOR, EDGE_SEPARATOR);
	exit(EXIT_FAILURE);
}

/**
 * handle parameters, calls usage(char*) for malformatted input
 * @brief parse argv for edges and verticies and write them to edges, verticies
 *
 * @param argc       Argument count
 * @param argv       Argument values
 * @param edge_num   Number of edges in argv starting at argv[optind]
 * @param edges      Array to store found edges to, must already be allocated
 * @param verticies  Array to store found verticies to, must already be allocated
 * @return           Number of verticies in input
**/
static int handleInput(int argc, char* argv[], int edge_num, edge_t* edges, int* verticies){
    for(int i=0; i<2*edge_num - 1; i++){
        verticies[i] = -1;
    }
    char edge_separator = EDGE_SEPARATOR;
    int vertex_num = 0;
    for(int i=0; i<edge_num; i++){
        //add edge
        char* firstVertex = strtok(argv[optind + i], &edge_separator);
        char* secondVertex = strtok(NULL, &edge_separator);
        if(strtok(NULL, &edge_separator) != NULL || firstVertex == NULL || secondVertex == NULL){
            usage(argv[0]);
        }
        char *end;
        int v1 = strtol(firstVertex, &end, 10);
        edges[i].v1 = v1;
        if(*end != '\0' || v1 < 0){
            usage(argv[0]);
        }
        int v2 = strtol(secondVertex, &end, 10);
        edges[i].v2 = v2;
        if(*end != '\0' || v2 < 0){
            usage(argv[0]);
        }

        //add verticies
        int* vertex = verticies;
        while(*vertex != -1) {
            //if verticies already contains v1 or v2 set it to -1
            if(*vertex == v1) {
                v1 = -1;
            }
            if(*vertex == v2) {
                v2 = -1;
            }
            vertex++;
        }
        //add if not set to -1 meaning they are not already contained in verticies
        if(v1 != -1){
            *vertex = v1;
            vertex_num++;
            vertex++;
        }
        if(v2 != -1){
            *vertex = v2;
            vertex_num++;
        }
    }
    return vertex_num;
}

/**
 * @brief find index of vertex in verticies[]

 * @param vertex      Vertex to search for
 * @param verticies   Array in which to search
 * @param vertex_num  Length of array
 * @return            Index of vertex in verticies
**/
static int indexOfVertex(int vertex, int verticies[], int vertex_num){
    for(int i=0; i<vertex_num; i++){
        if(verticies[i] == vertex) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief find a set of edges to remove for 3 coloring
 *
 * @param possibleSet3Coloring   Array in which to write the possible set of edges for 3 coloring, must already be allocated
 * @param verticies              Array of verticies
 * @param vertex_num             Number of verticies in verticies[]
 * @param edges                  Array of edges
 * @param edge_num               Number of edges in edges[]
 * @return                       Number of edges in possibleSet3Coloring
**/
static int selectPossibleSet3Coloring(edge_t *possibleSet3Coloring, int verticies[], int vertex_num, edge_t edges[], int edge_num){
    char vertexColors[vertex_num];
    for (int i=0; i<vertex_num; i++) {
        vertexColors[i] = rand() % 3;
    }
    int possibleSet3Coloring_num = 0;
    for(int i=0; i<edge_num; i++){
        int v1 = indexOfVertex(edges[i].v1, verticies, vertex_num);
        int v2 = indexOfVertex(edges[i].v2, verticies, vertex_num);
        if(v1 < 0 || v2 < 0){
            fprintf(stderr, "[%s] Vertex of Edge %d does not exist within Graph!\n", programname, i);
            exit(EXIT_FAILURE);
        }
        if(vertexColors[v1] == vertexColors[v2]){
            possibleSet3Coloring[possibleSet3Coloring_num] = edges[i];
            possibleSet3Coloring_num++;
        }
    }
    return possibleSet3Coloring_num;
}

/**
 * @brief Signal handler, sets quit flag to 1 to terminate program
 * @details Sets quit to 1
 *
 * @param signal   The signal for which this handler was called
**/
static void handle_signal(int signal){
    quit = 1;
}

/**
 * @brief Generate random but valid solutions to Minimal Set 3 Coloring where the graph is given via parameters (though they might not be minimal)

 * @param argc   Number of parameters
 * @param argv   No options, argv[1] and onward are edges
 * @return       Either EXIT_SUCCESS or EXIT_FAILURE
**/
int main(int argc, char* argv[]){
    //setup signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    //handle arguments
    programname = argv[0];
    int c;
    while((c = getopt(argc, argv, "")) != -1) {
        usage(argv[0]); //exit with usage message if options are used
    }
    if(argc - optind <= 0) {
        usage(argv[0]);
    }
    int edge_num = argc - optind;
    edge_t edges[edge_num];
    int verticies[2*edge_num - 1];
    int vertex_num = handleInput(argc, argv, edge_num, edges, verticies);

    //setup shared memory
    int fd_circular_buffer = shm_open(BUFFER_NAME, O_RDWR | O_CREAT, 0600);
    if(fd_circular_buffer == -1){
        fprintf(stderr, "[%s] Could not open circular buffer!\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int fd_terminate_flag  = shm_open(TERMINATE_FLAG_NAME, O_RDWR | O_CREAT, 0600);
    if(fd_terminate_flag == -1){
        fprintf(stderr, "[%s] Could not open circular buffer!\n", argv[0]);
        close(fd_circular_buffer);
        exit(EXIT_FAILURE);
    }
    struct sharedCircularBuffer *sharedCircularBuffer;
    sharedCircularBuffer = mmap(NULL, sizeof(*sharedCircularBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd_circular_buffer, 0);
    if(sharedCircularBuffer == MAP_FAILED) {
        fprintf(stderr, "[%s] Memory mapping failed!\n", argv[0]);
        close(fd_circular_buffer);
        close(fd_terminate_flag);
        exit(EXIT_FAILURE);
    }
    TERMINATE_FLAG_T *terminate_flag = mmap(NULL, sizeof(TERMINATE_FLAG_T), PROT_WRITE, MAP_SHARED, fd_terminate_flag, 0);
    if(terminate_flag == MAP_FAILED) {
        fprintf(stderr, "[%s] Memory mapping failed!\n", argv[0]);
        close(fd_circular_buffer);
        close(fd_terminate_flag);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        exit(EXIT_FAILURE);
    }
    if(close(fd_circular_buffer) == -1 || close(fd_terminate_flag) == -1){
        fprintf(stderr, "[%s] Closing file descriptor failed!\n", argv[0]);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        munmap(terminate_flag, sizeof(*terminate_flag));
        exit(EXIT_FAILURE);
    }
    //setup semaphores
    sem_t *sem_free = sem_open(SEM_FREE_BUFFER, 0);
    if(sem_free == SEM_FAILED){
        fprintf(stderr, "[%s] Error occurred while opening Semaphore %s: %s\n", argv[0], SEM_FREE_BUFFER, strerror(errno));
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        munmap(terminate_flag, sizeof(*terminate_flag));
        exit(EXIT_FAILURE);
    }
    sem_t *sem_used = sem_open(SEM_USED_BUFFER, 0);
    if(sem_used == SEM_FAILED){
        fprintf(stderr, "[%s] Error occurred while opening Semaphore %s: %s\n", argv[0], SEM_USED_BUFFER, strerror(errno));
        sem_close(sem_free);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        munmap(terminate_flag, sizeof(*terminate_flag));
        exit(EXIT_FAILURE);
    }
    sem_t *sem_wr = sem_open(SEM_WR_BUFFER, 0);
    if(sem_wr == SEM_FAILED){
        fprintf(stderr, "[%s] Error occurred while opening Semaphore %s: %s\n", argv[0], SEM_WR_BUFFER, strerror(errno));
        sem_close(sem_free);
        sem_close(sem_used);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        munmap(terminate_flag, sizeof(*terminate_flag));
        exit(EXIT_FAILURE);
    }
    //search for solutions
    int solution_length = INT_MAX;
    time_t t;
    srand((unsigned)time(&t)); //use the time as seed for random numbers
    edge_t minimalSet3Coloring[edge_num];
    int possibleSet3Coloring_length;
    while(solution_length > 0 && quit == 0 && *terminate_flag == 0){
        possibleSet3Coloring_length = selectPossibleSet3Coloring(possibleSet3Coloring, verticies, vertex_num, edges, edge_num);
        if(possibleSet3Coloring_length < solution_length){
            solution_length = possibleSet3Coloring_length;
            possibleSet3Coloring[solution_length].v1 = BUFFER_END_SYMBOL; //set last edge to {-1, -1}
            possibleSet3Coloring[solution_length].v2 = BUFFER_END_SYMBOL; //set last edge to {-1, -1}
            while(sem_wait(sem_wr) == -1 && quit == 0 && *terminate_flag == 0) { //set this process as writing to circular buffer
                if(errno != EINTR) {
                    fprintf(stderr, "[%s] Process interrupted while waiting: %s\n", argv[0], strerror(errno));
                    sem_close(sem_free);
                    sem_close(sem_used);
                    sem_close(sem_wr);
                    munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
                    exit(EXIT_FAILURE);
                }
            }
            //write solution to buffer
            for(int i=0; i <= solution_length; i++){
                while(sem_wait(sem_free) == -1 && quit == 0 && *terminate_flag == 0) {
                    if(errno != EINTR) {
                        fprintf(stderr, "[%s] Process interrupted while waiting: %s\n", argv[0], strerror(errno));
                        sem_close(sem_free);
                        sem_close(sem_used);
                        sem_close(sem_wr);
                        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
                        exit(EXIT_FAILURE);
                    }
                }
                sharedCircularBuffer->edges[sharedCircularBuffer->w].v1 = possibleSet3Coloring[i].v1;
                sharedCircularBuffer->edges[sharedCircularBuffer->w].v2 = possibleSet3Coloring[i].v2;
                sharedCircularBuffer->w = (sharedCircularBuffer->w + 1) % BUFFER_MAX_DATA;
                while(sem_post(sem_used) == -1 && quit == 0 && *terminate_flag == 0) {
                    if(errno != EINTR) {
                        fprintf(stderr, "[%s] Process interrupted while increasing semaphore: %s\n", argv[0], strerror(errno));
                        sem_close(sem_free);
                        sem_close(sem_used);
                        sem_close(sem_wr);
                        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
                        exit(EXIT_FAILURE);
                    }
                }
            }
            while(sem_post(sem_wr) == -1 && quit == 0 && *terminate_flag == 0) { //no longer writing to circular buffer
                if(errno != EINTR) {
                    fprintf(stderr, "[%s] Process interrupted while increasing semaphore: %s\n", argv[0], strerror(errno));
                    sem_close(sem_free);
                    sem_close(sem_used);
                    sem_close(sem_wr);
                    munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    //cleanup
    //cleanup semaphores
    if(sem_close(sem_free) == -1) {
        fprintf(stderr, "[%s] Error occurred while closing semaphore: %s\n", argv[0], strerror(errno));
        sem_close(sem_used);
        sem_close(sem_wr);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        munmap(terminate_flag, sizeof(*terminate_flag));
        exit(EXIT_FAILURE);
    }
    if(sem_close(sem_used) == -1) {
        fprintf(stderr, "[%s] Error occurred while closing semaphore: %s\n", argv[0], strerror(errno));
        sem_close(sem_wr);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        munmap(terminate_flag, sizeof(*terminate_flag));
        exit(EXIT_FAILURE);
    }
    if(sem_close(sem_wr) == -1) {
        fprintf(stderr, "[%s] Error occurred while closing semaphore: %s\n", argv[0], strerror(errno));
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        munmap(terminate_flag, sizeof(*terminate_flag));
        exit(EXIT_FAILURE);
    }
    //cleanup shared memory
    if(munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer)) == -1){
        fprintf(stderr, "[%s] Unmapping Memory failed!\n", argv[0]);
        munmap(terminate_flag, sizeof(*terminate_flag));
        exit(EXIT_FAILURE);
    }
    if(munmap(terminate_flag, sizeof(*terminate_flag)) == -1){
        fprintf(stderr, "[%s] Unmapping Memory failed!\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);

}

