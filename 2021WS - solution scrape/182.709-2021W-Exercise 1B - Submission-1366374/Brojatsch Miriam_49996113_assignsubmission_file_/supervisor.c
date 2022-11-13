/**
* @file supervisor.c
* @author Miriam Brojatsch 11913096
* @date 14.11.2021
*
* @brief This is a supervisor who sets up shared memory and semaphores, reads solutions of generator from circular buffer and remembers the best one so far. Handles interrupt signals and cleans up after itself;
**/

#include "general.h"

//program name
static const char* NAME;

/**
* @brief Signal handler sets please_exit flag in buffer to 1 for generator to read;
* @param signal: a signal
**/
void handle_signal(int signal) {
    buffer->please_exit = 1;
}

/**
* @brief Synchronized with semaphores, reads buffer at read_at and returns that solution_set. Posts semaphores afterwards, naturally.
**/
static solution_set read_buffer(void) {
    if(sem_wait(sem_used) == -1){
        if(errno != EINTR){
            fprintf(stderr, "Error occured while waiting to read in %s", NAME);
            exit(EXIT_FAILURE);
        }
    }
    solution_set candidate = buffer->data[buffer->read_at];
    buffer->read_at = (buffer->read_at + 1)%BUFFER_SIZE;

    if(sem_post(sem_free) == -1){
        fprintf(stderr, "Error while calling sem_post in %s", NAME);
        exit(EXIT_FAILURE);
    }
    return candidate;
}


/**
* @brief Main function of supervisor. Checks arguments, sets up signal handler, shared memory, mapping, semaphores, then while in a loop reads the buffer and checks for best solutions
* until either there's an interrupt or a generator found out it is already an acyclic graph. Cleans up after itself as well.
* @param argc: argument counter
* @param argv: arguments of main (must be none except for program name)
**/
int main(int argc, char **argv) {

    NAME = argv[0];

    if (argc > 1) {
        fprintf(stderr, "Usage: ./supervisor\n");
        exit(EXIT_FAILURE);
    }

    //set up signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);


    solution_set best_so_far;
    best_so_far.size = 9; //solutions proposed by generators are 8 at max, therefore guaranteed to be better than this
    for (int i = i; i < 8; ++i) { //initialize with 0 so if no better solution is found at least the output makes no sense
        best_so_far.edges[i].u = 0;
        best_so_far.edges[i].u = 0;
    }

    solution_set candidate;

    //create shared memory
    shm_fd = shm_open(MATRICULATION_NO, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm_fd == -1) { //because either shm_open failed or was not called
        fprintf(stderr, "Shared memory could not be created in %s\n", NAME);
        exit(EXIT_FAILURE);
    }

    //set size
    if (ftruncate(shm_fd, sizeof(circular_buffer)) == -1) {
        fprintf(stderr, "Shared memory size could not be set in %s\n", NAME);
        close(shm_fd);
        shm_unlink(MATRICULATION_NO);
        exit(EXIT_FAILURE);
    }


    //create mapping
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buffer == MAP_FAILED) {
        fprintf(stderr, "Mapping could not be created in %s\n", NAME);
        close(shm_fd);
        shm_unlink(MATRICULATION_NO);
        exit(EXIT_FAILURE);
    }


    //open semaphores
    sem_free = sem_open(SEM_FREE_NAME,O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    if (sem_free == SEM_FAILED) {
        fprintf(stderr, "Could not create sem_free %s", NAME);
        exit(EXIT_FAILURE);
    }
    sem_used = sem_open(SEM_USED_NAME, O_CREAT | O_EXCL, 0600, 0);
    if (sem_used == SEM_FAILED) {
        fprintf(stderr, "Could not create sem_used %s", NAME);
        exit(EXIT_FAILURE);
    }
    sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT | O_EXCL, 0600, 1);
    if (sem_mutex == SEM_FAILED) {
        fprintf(stderr, "Could not create sem_mutex in %s", NAME);
        exit(EXIT_FAILURE);
    }

    //initialize buffer
    buffer->read_at = 0;
    buffer->write_at = 0;
    buffer->please_exit = 0;
    



    //wait for generators to write in circular buffer
    while (buffer->please_exit == 0) {
        candidate = read_buffer(); //read solution from buffer and remember best one
        if (candidate.size == 0) {
            printf("The graph is acyclic!\n"); //if new solution has 0 edges print that graph is acyclic and terminate 
            buffer->please_exit = 1;
        }
        if (candidate.size < best_so_far.size) { //if new solution is better than last one, print to stdout
            best_so_far = candidate;
            printf("Solution with %d edges", best_so_far.size);
            for (int i = 0; i < best_so_far.size; ++i) {
                printf(" %d-%d", best_so_far.edges[i].u, best_so_far.edges[i].v);
            }
            printf("\n");
        }
        //else keep reading until sigint or sigterm or acyclic
    }

    
    //cleanup
    munmap(buffer, sizeof( * buffer));
    close(shm_fd);
    shm_unlink(MATRICULATION_NO);
    sem_close(sem_free);
    sem_close(sem_used);
    sem_close(sem_mutex);
    sem_unlink(SEM_FREE_NAME);
    sem_unlink(SEM_USED_NAME);
    sem_unlink(SEM_MUTEX_NAME);

    exit(EXIT_SUCCESS);
}