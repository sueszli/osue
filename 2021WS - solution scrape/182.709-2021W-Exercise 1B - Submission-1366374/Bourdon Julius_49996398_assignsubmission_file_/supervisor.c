/**
 * @file supervisor.c
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 14.11.2022
 *
 * @brief Implementation of the supervisor module (part of the fb_arc_set functionality)
 * 
 * @details The supervisor sets up the shared memory as well as the semaphores necessary for synchronisation with the generators.
 *          During execution the supervisor checks the generated solutions by the generators and remembers the best solution. If the  
 *          supervisor finds a solution with 0 edges in the shared memory , i.e. the given graph is acyclic, or it receives a SIGINT or  
 *          a SIGTERM signal, then it informs all the generators to terminate (via a quit-flag in the shared memory).
 *          After that it frees the shared memory and the used semaphores and terminates as well.  
 *      
 **/


#include "fb_arc_set.h"

/** This global variable defines whether the supervisor-loop is terminated (when set to 1) */
volatile sig_atomic_t quit_flag_supervisor = 0;

/**
 * @brief This static function prints information about the correct use of the supervisor program.
**/
void static usage_message(void){
    fprintf(stderr, "Usage: supervisor\n");
}

/**
 * @brief This static function sets the global variable quit_flag_supervisor.
**/
void static set_quit_flag_supervisor(int _){
    quit_flag_supervisor = 1;
}

/**
 * @brief This static function initializes a sigaction, which is executed when the program receives the specified signal.
 *
 * @param signal     The specified signal.
 * @param sighandler The pointer to the function to be executed after receiving the specified signal.
 * @param sigact     The pointer to the sigaction to be initialized.
 */
void static initialize_sigaction(int signal, struct sigaction *sigact, void(*sighandler)(int)){
    memset(sigact, 0, sizeof(*sigact));
    (*sigact).sa_handler = sighandler;
    sigaction(signal, sigact, NULL); 
}

/**
 * @brief This static function does the cleanup of the shared resourcs (closing and removing of shared memory + semaphores)
 *
 * @param circ_buffer_used  The circular buffer which was used in the shared memory to do the communication between supervisor and generators.
 * @param fd_shm            The file descriptor of the shared memory object.
 * @param used_sem          The pointer to the semaphore tracking the used ('ready-to-read') space in the circular buffer. 
 * @param free_sem          The pointer to the semaphore tracking the free ('ready-to-write') space in the circular buffer.
 * @param mutex_sem         The pointer to the semaphore handling mutual exclusion betweeen the various generators writing in the circular buffer.
 * @param close_sem         The pointer to the semaphore ensuring the closing of the generator happens before the closing of the supervisor
 *                          (only relevant if there is just one generator).
 */
void static inline cleanup(circular_buffer_t *circ_buffer_used, int fd_shm, sem_t *used_sem, sem_t *free_sem, sem_t *mutex_sem, sem_t *close_sem){
    
    circ_buffer_used->quit_flag_generators = true;

    int i;
    for(i = 0; i < 10; i++){
        sem_post(free_sem);
    }
    
    /* NOTE: workaround for the problem with closing the semaphore and the shared memory (assumes there is just one generator) */
    sem_wait(close_sem);

    remove_sem_as_server(used_sem, USED_SEM_NAME);
    remove_sem_as_server(free_sem, FREE_SEM_NAME);
    remove_sem_as_server(mutex_sem, MUTEX_SEM_NAME);

    /* NOTE: workaround vor the problem with closing the semaphore and the shared memory (assumes there is just one generator) */
    remove_sem_as_server(close_sem, "/11904658_sem_close");

    close_shared_mem_as_server(fd_shm, circ_buffer_used);
}

/**
 * @brief This static function does the same as the static function 'cleanup', plus it calls exit(3). It furthermore has the same 
 *        parameters as 'cleanup'.
 */
void static inline cleanup_and_exit(circular_buffer_t *circ_buffer_used, int fd_shm, sem_t *used_sem, sem_t *free_sem, sem_t *mutex_sem, sem_t *close_sem){

    cleanup(circ_buffer_used, fd_shm, used_sem, free_sem, mutex_sem, close_sem);
    exit(EXIT_FAILURE);
}


/**
 * @brief   The main function of the supervisor program.
 * @details The main function is responsible for the execution of the inherent functionality of the program.
 *          It calls other functions to initialize the shared memory, the sigaction-struct and the semaphores. 
 *          Furthermore it opens a loop, in which it continously checks the circular buffer for new entries from various
 *          generators and remembers the best solution found so far.
 *
 * @param argc The argument counter. It must always be one as no options or positional arguments are allowed.
 * @param argv The array/pointer to the argument values. It must only contain the program name.
 *
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[]){

    if(argc != 1){
        fprintf(stderr, "'supervisor' does not allow any options or positional arguments!\n");
        usage_message();
        exit(EXIT_FAILURE);
    }

    
    struct sigaction sigaction_for_SIGINT;
    struct sigaction sigaction_for_SIGTERM;
    initialize_sigaction(SIGINT, &sigaction_for_SIGINT, set_quit_flag_supervisor);
    initialize_sigaction(SIGTERM, &sigaction_for_SIGTERM, set_quit_flag_supervisor);

    int fd_shm;
    circular_buffer_t *circ_buffer_used;
    initialize_shared_mem_as_server(&fd_shm, &circ_buffer_used);

    sem_t *used_sem;
    sem_t *free_sem;
    sem_t *mutex_sem;

    initialize_sem_as_server(&used_sem, USED_SEM_NAME, USED_SEM_INIT_VALUE);
    initialize_sem_as_server(&free_sem, FREE_SEM_NAME, FREE_SEM_INIT_VALUE);
    initialize_sem_as_server(&mutex_sem, MUTEX_SEM_NAME, MUTEX_SEM_INIT_VALUE);

    /* NOTE: workaround for the problem with closing the semaphore and the shared memory (assumes there is just one generator) */
    sem_t *close_sem;
    initialize_sem_as_server(&close_sem, "/11904658_sem_close", 0);
    
    /* NOTE: nur um den Semaphor-Startwert zu überprüfen */
    int value_used_sem;
    int value_free_sem;
    int value_mutex_sem;
    int value_close_sem;
    sem_getvalue(used_sem, &value_used_sem);
    sem_getvalue(free_sem, &value_free_sem);
    sem_getvalue(mutex_sem, &value_mutex_sem);
    sem_getvalue(close_sem, &value_close_sem);
    fprintf(stdout, "\nChecking of correct sem-values:\n");
    fprintf(stdout, "value used_sem: %d\n", value_used_sem);
    fprintf(stdout, "value free_sem: %d\n", value_free_sem);
    fprintf(stdout, "value mutex_sem: %d\n", value_mutex_sem);
    fprintf(stdout, "value close_sem: %d\n", value_close_sem);
    /* END NOTE! */
    
    fprintf(stdout, "\ncircular buffer created\n");

    struct edge_set *taken_set;
    struct edge_set current_best_solution = {.length = 9};

    //sem_post(close_sem);

    /* NOTE: loop to keep the supervisor running while testing */
    while(quit_flag_supervisor != 1){

        if(sem_wait(used_sem) == -1){
            if(errno == EINTR){
                continue;
            }
            fprintf(stderr, "Prog. 'supervisor' - sem_wait failed: %s\n", strerror(errno));
            cleanup_and_exit(circ_buffer_used, fd_shm, used_sem, free_sem, mutex_sem, close_sem);
        }

        taken_set = &(circ_buffer_used->buffer_array[circ_buffer_used->read_end]);
        
        if(taken_set->length < current_best_solution.length){

            current_best_solution.length = taken_set->length;

            if(current_best_solution.length == 0){
                fprintf(stdout, "\nThe given graph is acylic!\n");

                // unelegante Lösung vom Loop-Problem bei mehreren Generatoren
                if(sem_post(free_sem) == -1){
                    fprintf(stderr, "Prog. 'supervisor' - sem_post failed: %s\n", strerror(errno));
                    cleanup_and_exit(circ_buffer_used, fd_shm, used_sem, free_sem, mutex_sem, close_sem);
                }
                if(sem_post(free_sem) == -1){
                    fprintf(stderr, "Prog. 'supervisor' - sem_post failed: %s\n", strerror(errno));
                    cleanup_and_exit(circ_buffer_used, fd_shm, used_sem, free_sem, mutex_sem, close_sem);
                }                
                if(sem_post(free_sem) == -1){
                    fprintf(stderr, "Prog. 'supervisor' - sem_post failed: %s\n", strerror(errno));
                    cleanup_and_exit(circ_buffer_used, fd_shm, used_sem, free_sem, mutex_sem, close_sem);
                }


                set_quit_flag_supervisor(1);
            } else {               
                memcpy(current_best_solution.edge_array, taken_set->edge_array, sizeof(struct edge)*MAX_NO_EDGES_ALLOWED_IN_SOLUTION);
                print_edge_set(&current_best_solution, "\nCurrent best solution");
            }
        }

        circ_buffer_used->read_end = (circ_buffer_used->read_end+1) % BUFFER_SIZE;


        if(sem_post(free_sem) == -1){
            fprintf(stderr, "Prog. 'supervisor' - sem_post failed: %s\n", strerror(errno));
            cleanup_and_exit(circ_buffer_used, fd_shm, used_sem, free_sem, mutex_sem, close_sem);
        }

    }

    cleanup(circ_buffer_used, fd_shm, used_sem, free_sem, mutex_sem, close_sem);

    fprintf(stdout, "\ncircular buffer unlinked!\n");

    return EXIT_SUCCESS;

}