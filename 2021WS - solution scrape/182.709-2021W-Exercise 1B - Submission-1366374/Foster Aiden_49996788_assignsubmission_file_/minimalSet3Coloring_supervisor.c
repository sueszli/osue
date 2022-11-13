/**
 * @file minimalSet3Coloring_supervisor.c
 * @author Aiden Foster 11910604
 * @date 12.11.2021
 *
 * @brief supervisor compares the solutions for Minimal Set 3 Coloring provided by generators and finds their minimum
 * @details: quit is a global variable that can be set within the signal handler to tell the programm to terminate
**/

#include "minimalSet3Coloring_supervisor.h"

/**
 * @brief Quit flag, this is set to 1 when the program should terminate
**/
volatile sig_atomic_t quit = 0; //global variable to determine when to stop

/**
 * usage function
 * @brief Output a usage message to stderr and terminate
**/
static void usage(char* programname){
	fprintf(stderr, "Usage: %s\n", programname);
	exit(EXIT_FAILURE);
}

/**
 * @brief Prints an edge to stdout
 *
 * @param edge Edge to print
**/
static void print_edge(edge_t edge){
    printf(" %d-%d", edge.v1, edge.v2);
}

/**
 * @brief Signal handler, sets quit flag to 1 to terminate program
 * @details Sets quit to 1
**/
static void handle_signal(int signal){
    quit = 1;
}

/**
 * @brief starts the supervisor that compares solutions from multiple generators to find their minimum
 *
 * @param argc   No further arguments expected
 * @param argv   No further arguments expected
 * @return       Either EXIT_SUCCESS or EXIT_FAILURE
**/
int main(int argc, char* argv[]){
    //setup signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    //handle arguments (should have none)
    int c;
    while((c = getopt(argc, argv, "")) != -1) {
        usage(argv[0]); //exit with usage message if options are used
    }
    if(argc - optind > 0) {
        usage(argv[0]);
    }
    //setup shared memory
    //circular buffer
    int fd_circular_buffer = shm_open(BUFFER_NAME, O_RDWR | O_CREAT, 0600);
    if(fd_circular_buffer == -1){
        fprintf(stderr, "[%s] Could not open circular buffer!\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //terminate flag for generators
    int fd_terminate_flag  = shm_open(TERMINATE_FLAG_NAME, O_RDWR | O_CREAT, 0600);
    if(fd_terminate_flag == -1){
        fprintf(stderr, "[%s] Could not open circular buffer!\n", argv[0]);
        close(fd_circular_buffer);
        exit(EXIT_FAILURE);
    }
    if(ftruncate(fd_circular_buffer, sizeof(struct sharedCircularBuffer)) < 0){ //set size of shared memory
        fprintf(stderr, "[%s] Could not set size of circular buffer!\n", argv[0]);
        close(fd_circular_buffer);
        close(fd_terminate_flag);
        exit(EXIT_FAILURE);
    }
    if(ftruncate(fd_terminate_flag, sizeof(TERMINATE_FLAG_T)) < 0){ //set size of shared memory
        fprintf(stderr, "[%s] Could not set size of circular buffer!\n", argv[0]);
        close(fd_circular_buffer);
        close(fd_terminate_flag);
        exit(EXIT_FAILURE);
    }
    TERMINATE_FLAG_T *terminate_flag = mmap(NULL, sizeof(TERMINATE_FLAG_T), PROT_WRITE, MAP_SHARED, fd_terminate_flag, 0);
    if(terminate_flag == MAP_FAILED) {
        fprintf(stderr, "[%s] Memory mapping failed!\n", argv[0]);
        close(fd_circular_buffer);
        close(fd_terminate_flag);
        exit(EXIT_FAILURE);
    }
    struct sharedCircularBuffer *sharedCircularBuffer;
    sharedCircularBuffer = mmap(NULL, sizeof(*sharedCircularBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd_circular_buffer, 0);
    if(sharedCircularBuffer == MAP_FAILED) {
        fprintf(stderr, "[%s] Memory mapping failed!\n", argv[0]);
        *terminate_flag = 1; //tell generators to terminate
        close(fd_circular_buffer);
        close(fd_terminate_flag);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(close(fd_circular_buffer) == -1 || close(fd_terminate_flag) == -1){
        fprintf(stderr, "[%s] Closing file descriptor failed!\n", argv[0]);
        *terminate_flag = 1; //tell generators to terminate
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    //shared memory setup complete
    //setup semaphores
    sem_t *sem_free = sem_open(SEM_FREE_BUFFER, O_CREAT | O_EXCL, 0600, BUFFER_MAX_DATA);
    if(sem_free == SEM_FAILED) {
        fprintf(stderr, "[%s] Error occurred while opening Semaphore %s: %s\n", argv[0], SEM_FREE_BUFFER, strerror(errno));
        *terminate_flag = 1; //tell generators to terminate
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    sem_t *sem_used = sem_open(SEM_USED_BUFFER, O_CREAT | O_EXCL, 0600, 0);
    if(sem_used == SEM_FAILED) {
        fprintf(stderr, "[%s] Error occurred while opening Semaphore %s: %s\n", argv[0], SEM_FREE_BUFFER, strerror(errno));
        *terminate_flag = 1; //tell generators to terminate
        sem_close(sem_free);
        sem_unlink(SEM_FREE_BUFFER);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    sem_t *sem_client = sem_open(SEM_WR_BUFFER, O_CREAT | O_EXCL, 0600, 1); //one client is allowed to write at a time
    if(sem_client == SEM_FAILED) {
        fprintf(stderr, "[%s] Error occurred while opening Semaphore %s: %s\n", argv[0], SEM_FREE_BUFFER, strerror(errno));
        *terminate_flag = 1; //tell generators to terminate
        sem_close(sem_free);
        sem_close(sem_used);
        sem_unlink(SEM_FREE_BUFFER);
        sem_unlink(SEM_USED_BUFFER);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    int solution_length = INT_MAX;
    edge_t* minimalSet = (edge_t *) malloc(BUFFER_MAX_DATA * sizeof(edge_t));
    edge_t tmp;
    int index = 0;
    int max_index = BUFFER_MAX_DATA;
    while(solution_length > 0 && quit == 0){
        while(sem_wait(sem_used) == -1 && quit == 0) { //wait for and decrease space in circular buffer
            if(errno != EINTR) { //do not terminate due to EINTR
                fprintf(stderr, "[%s] Process interrupted while waiting: %s\n", argv[0], strerror(errno));
                *terminate_flag = 1; //tell generators to terminate
                sem_close(sem_free);
                sem_close(sem_used);
                munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
                shm_unlink(BUFFER_NAME);
                munmap(terminate_flag, sizeof(*terminate_flag));
                shm_unlink(TERMINATE_FLAG_NAME);
                free(minimalSet);
                exit(EXIT_FAILURE);
            }
        }
        tmp.v1 = sharedCircularBuffer->edges[sharedCircularBuffer->r].v1;
        tmp.v2 = sharedCircularBuffer->edges[sharedCircularBuffer->r].v2;
        sharedCircularBuffer->r = (sharedCircularBuffer->r + 1) % BUFFER_MAX_DATA;
        while(sem_post(sem_free) == -1 && quit == 0){ //increase free amount in circular buffer
            if(errno != EINTR) {
                fprintf(stderr, "[%s] Process interrupted while increasing semaphore: %s\n", argv[0], strerror(errno));
                *terminate_flag = 1; //tell generators to terminate
                sem_close(sem_free);
                sem_close(sem_used);
                munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
                shm_unlink(BUFFER_NAME);
                munmap(terminate_flag, sizeof(*terminate_flag));
                shm_unlink(TERMINATE_FLAG_NAME);
                free(minimalSet);
                exit(EXIT_FAILURE);
            }
        }
        if(index < solution_length) {
            minimalSet[index].v1 = tmp.v1;
            minimalSet[index].v2 = tmp.v2;
        }
        if(tmp.v1 == BUFFER_END_SYMBOL || tmp.v2 == BUFFER_END_SYMBOL){
            if(index < solution_length){
                solution_length = index;
                if(solution_length >= MIN_REALLOC_SIZE) {
                    max_index = solution_length;
                    minimalSet = realloc(minimalSet, solution_length * sizeof(edge_t)); //save memory when solution shrinks
                }
                if (solution_length == 0) {
                    printf("[%s] The graph is 3-colorable!\n", argv[0]);
                } else {
                    printf("[%s] Solution with %d edges:", argv[0], solution_length);
                    for(int i=0; i<solution_length; i++){
                        print_edge(minimalSet[i]);
                    }
                    printf("\n");
                }
            }
            index = -1;
        } else if(index >= max_index-1) { //increase array size if to small
            max_index *= 2;
            minimalSet = realloc(minimalSet, max_index * sizeof(edge_t));
        }
        index++;
    }
    //cleanup
    *terminate_flag = 1; //terminate generators
    free(minimalSet);
    minimalSet = NULL;
    //cleanup semaphores
    if(sem_close(sem_free) == -1) {
        fprintf(stderr, "[%s] Error occurred while closing semaphore: %s\n", argv[0], strerror(errno));
        sem_close(sem_used);
        sem_close(sem_client);
        sem_unlink(SEM_FREE_BUFFER);
        sem_unlink(SEM_USED_BUFFER);
        sem_unlink(SEM_WR_BUFFER);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(sem_close(sem_used) == -1) {
        fprintf(stderr, "[%s] Error occurred while closing semaphore: %s\n", argv[0], strerror(errno));
        sem_close(sem_client);
        sem_unlink(SEM_FREE_BUFFER);
        sem_unlink(SEM_USED_BUFFER);
        sem_unlink(SEM_WR_BUFFER);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(sem_close(sem_client) == -1) {
        fprintf(stderr, "[%s] Error occurred while closing semaphore: %s\n", argv[0], strerror(errno));
        sem_unlink(SEM_FREE_BUFFER);
        sem_unlink(SEM_USED_BUFFER);
        sem_unlink(SEM_WR_BUFFER);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(sem_unlink(SEM_FREE_BUFFER) == -1) {
        fprintf(stderr, "[%s] Error occurred while unlinking semaphore: %s\n", argv[0], strerror(errno));
        sem_unlink(SEM_USED_BUFFER);
        sem_unlink(SEM_WR_BUFFER);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(sem_unlink(SEM_USED_BUFFER) == -1) {
        fprintf(stderr, "[%s] Error occurred while unlinking semaphore: %s\n", argv[0], strerror(errno));
        sem_unlink(SEM_WR_BUFFER);
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(sem_unlink(SEM_WR_BUFFER) == -1) {
        fprintf(stderr, "[%s] Error occurred while unlinking semaphore: %s\n", argv[0], strerror(errno));
        munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer));
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    //cleanup shared memory
    if(munmap(sharedCircularBuffer, sizeof(*sharedCircularBuffer)) == -1){
        fprintf(stderr, "[%s] Unmapping Memory failed!\n", argv[0]);
        shm_unlink(BUFFER_NAME);
        munmap(terminate_flag, sizeof(*terminate_flag));
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(munmap(terminate_flag, sizeof(*terminate_flag)) == -1){
        fprintf(stderr, "[%s] Unmapping Memory failed!\n", argv[0]);
        shm_unlink(BUFFER_NAME);
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(shm_unlink(BUFFER_NAME) == -1) {
        fprintf(stderr, "[%s] Unlinking buffer failed!\nYou can remove it in /dev/shm\n", argv[0]);
        shm_unlink(TERMINATE_FLAG_NAME);
        exit(EXIT_FAILURE);
    }
    if(shm_unlink(TERMINATE_FLAG_NAME) == -1) {
        fprintf(stderr, "[%s] Unlinking terminate flag failed!\nYou can remove it in /dev/shm\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

