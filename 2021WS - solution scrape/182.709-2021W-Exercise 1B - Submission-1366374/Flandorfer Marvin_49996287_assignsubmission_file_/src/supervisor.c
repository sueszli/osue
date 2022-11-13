/**
 * Supervisor module
 * @file supervisor.c
 * @author Marvin Flandorfer, 52004069
 * @date 08.11.2021
 * 
 * @brief This is the supervisor module.
 * This module is the main entry point for the supervisor program and should be startet before the generator program.
 * 
 * @details This module is responsible for the supervision of all running generator programs.
 * The shared memory as well as all semaphores are created here and will be closed and destroyed here.
 * In this module, the supervisor reads from the shared memory and stores the best solution so far.
 * It terminates if either the graph is acyclic or it gets interrupted by a SIGINT or SIGTERM signal.
 * Before it terminates, all generators get notified so that they can terminate as well.
 */

#include <signal.h>
#include <errno.h>
#include "misc.h"
#include "list.h"
#include "shmsupvis.h"
#include "semaphores.h"

char *program_name; /**< The program name*/
volatile sig_atomic_t quit = 0; /**< Global variable that signals the program to terminate, if it gets interrupted by SIGINT or SIGTERM*/

/**
 * Function for signal handling
 * @brief This function sets the global quit variable to 1.
 * @details If the program gets interrupted by a SIGINT or SIGTERM signal, this function gets called and quit will be set to 1.
 * @details Global variables: quit
 * 
 * @param signal Represents the code of the signal.
 */
static void handle_signal(int signal){
    if(signal == SIGINT || signal == SIGTERM){
        quit = 1;
    }
}

/**
 * Function for reading from buffer.
 * @brief This function reads one integer from the circular buffer in the shared memory and returns it.
 * @details After waiting for an unlocked semaphore, the function decreases the writing semaphore and reads from the buffer.
 * After reading, the read pointer gets incremented and will be set to 0 if the pointer is greater than the size of the buffer.
 * 
 * @param shm Pointer to the shared memory.
 * @param free Pointer to the free space semaphore.
 * @param used Pointer to the used space semaphore.
 * @return Returns integer read from the circular buffer on success. 
 * If the process gets interrupted by a SIGINT or SIGTERM signal, returns -4.
 * If an error occurs (that is not EINTR) returns -3.
 * If this is the end of the current solution is reached, -2 will be returned.
 * If the graph is acyclic, -1 will be returned.
 */
static int read_from_buffer(shm_t *shm, sem_t *free, sem_t *used){
    if(sem_wait(used) < 0){
        if(errno == EINTR){
            return -4;
        }
        error_message("sem_wait");
        return -3;
    }
    int val = shm->buffer[shm->read_pos]; /**< Integer from the circular buffer*/
    if(sem_post(free) < 0){
        if(errno == EINTR){
            return -4;
        }
        error_message("sem_post");
        return -3;
    }
    shm->read_pos++;
    shm->read_pos %= sizeof(shm->buffer)/sizeof(int);
    return val;
}

/**
 * Function for closing and unlinking all semaphores
 * @brief This function closes all semaphores and subsequently unlinks all semaphores.
 * @details All three semaphores (SEM_FREE_SPACE, SEM_USED_SPACE and SEM_EXCL_ACCESS) will be closed.
 * After that and after all other programs have close these semaphores, they will be unlinked.
 * 
 * @param free Pointer that points to the free space semaphore.
 * @param used Pointer that points to the used space semaphore.
 * @param access Pointer that points to the exclusive access semaphore.
 * @return On success returns 0. On failure due to an error returns -1.
 */
static int close_and_unlink_all_semaphores(sem_t *free, sem_t *used, sem_t *access){
    if(sem_close(free) < 0){
        error_message("sem_close");
        (void) sem_close(used);
        (void) sem_close(access);
        (void) sem_unlink(SEM_USED_SPACE);
        (void) sem_unlink(SEM_EXCL_ACCESS);
        return -1;
    }
    if(sem_close(used) < 0){
        error_message("sem_close");
        (void) sem_close(access);
        (void) sem_unlink(SEM_FREE_SPACE);
        (void) sem_unlink(SEM_EXCL_ACCESS);
        return -1;
    }
    if(sem_close(access) < 0){
        error_message("sem_close");
        (void) sem_unlink(SEM_USED_SPACE);
        (void) sem_unlink(SEM_FREE_SPACE);
        return -1;
    }
    if(sem_unlink(SEM_FREE_SPACE) < 0){
        error_message("sem_unlink");
        (void) sem_unlink(SEM_USED_SPACE);
        (void) sem_unlink(SEM_EXCL_ACCESS);
        return -1;
    }
    if(sem_unlink(SEM_USED_SPACE) < 0){
        error_message("sem_unlink");
        (void) sem_unlink(SEM_EXCL_ACCESS);
        return -1;
    }
    if(sem_unlink(SEM_EXCL_ACCESS) < 0){
        error_message("sem_unlink");
        return -1;
    }
    return 0;
}

/**
 * Function that prints a solution
 * @brief This function prints a solution (= feedback arc set) to stdout.
 * @details If the graph is acyclic, there will be no feedback arc set, but instead "The graph is acyclic!" will be printed.
 * If there is at least on edge, the solution will be printed in the following format:
 * "Solution with [NUMBER OF EDGES] edges: [EDGES...]"
 * 
 * @param solution Pointer that points to the head of an edge list (in this case the currently best solution).
 * @return Returns 0 on success. If due to an error failure occures, -1 will be returned.
 */
static int print_solution(edge_t *solution){
    int count = 0;                              /**< Counter that counts the number of edges*/
    edge_t *cur = solution;                     /**< Pointer to the head of the solution*/
    if(cur->from == -1){
        if(fprintf(stdout, "[%s] The graph is acyclic!\n", program_name) < 0){
            error_message("fprintf");
            return -1;
        }
        return 0;
    }
    while(cur != NULL){
        count++;
        cur = cur->next;
    }
    cur = solution;
    if(fprintf(stdout,"[%s] Solution with %i edges: ", program_name, count) < 0){
        error_message("fprintf");
        return -1;
    }
    while(cur != NULL){
        int a = fprintf(stdout, "%i-%i ", cur->from, cur->to);
        if(a < 0){
            error_message("fprintf");
            return -1;
        }
        cur = cur->next;
    }
    if(fprintf(stdout,"\n") < 0){
        error_message("fprintf");
        return -1;
    }
    return 0;
}

/**
 * Function for getting solutions from the circular buffer
 * @brief This function continously looks for solutions in the circular buffer and prints the best solution.
 * @details As long as either the graph is not determined to be acyclic or the quit variable is not 1 this function looks for solutions.
 * A solution gets constructed from the buffer in the following manner:
 * Values get read from the buffer pairwise. The first value is the node where the edge originates and the second value the node where the edge ends.
 * This goes on as long as the first value is not equal -2. If the value is -2, the solution will be concluded and compaired to the current best solution.
 * If the first value equals -1, the graph is acyclic and the program can be terminated.
 * If one of the values equals -3, an error occured in the read_from_buffer function. 
 * If one of teh values equals -4, the program got interrupted with eiter SIGINT or SIGTERM and will be terminated accordingly.
 * @details Global variables: quit
 * 
 * @param shm Pointer that points to the shared memory.
 * @param free Pointer that points to the free space semaphore.
 * @param used Pointer that points to the used space semaphore.
 * @return Returns 0 if either the graph is acyclic or the program got interrupted using SIGINT or SIGTERM.
 * Returns -1 on failure due to an error.
 */
static int get_best_solution(shm_t *shm, sem_t *free, sem_t *used){
    int best_count = 9;                     /**< Counter that counts length of the best solution*/
    edge_t *best_solution = NULL;           /**< Pointer that points to the head of the best solution*/
    while(!quit){
        int val1;                           /**< The node where the edge originates (read from circular buffer)*/
        int val2;                           /**< The node where the edge ends (read from the circular buffer)*/
        int cur_count = 0;                  /**< Counter that counts the length of the current solution*/
        edge_t *cur_solution = NULL;        /**< Pointer that points to the head of the current solution*/
        while((val1 = read_from_buffer(shm, free, used)) != -2){
            if(val1 == -1){
                free_all_list_nodes(best_solution);
                free_all_list_nodes(cur_solution);
                best_solution = init_head(-1,-1);
                if(best_solution == NULL){
                    return -1;
                }
                if(print_solution(best_solution) < 0){
                    return -1;
                }
                free_all_list_nodes(best_solution);
                return 0;
            }
            if(val1 == -3){
                free_all_list_nodes(cur_solution);
                free_all_list_nodes(best_solution);
                return -1;
            }
            if(val1 == -4){
                free_all_list_nodes(cur_solution);
                free_all_list_nodes(best_solution);
                return 0;
            }
            val2 = read_from_buffer(shm, free, used);
            if(val2 == -3){
                free_all_list_nodes(cur_solution);
                free_all_list_nodes(best_solution);
                return -1;
            }
            if(val2 == -4){
                free_all_list_nodes(cur_solution);
                free_all_list_nodes(best_solution);
                return 0;
            }
            if(cur_solution == NULL){
                cur_solution = init_head(val1, val2);
                if(cur_solution == NULL){
                    free_all_list_nodes(best_solution);
                    return -1;
                }
            }
            else{
                if(push(cur_solution, val1, val2) < 0){
                    free_all_list_nodes(cur_solution);
                    free_all_list_nodes(best_solution);
                    return -1;
                }
            }
            cur_count++;
        }
        if(cur_count < best_count){
            best_count = cur_count;
            free_all_list_nodes(best_solution);
            best_solution = cur_solution;
            if(print_solution(best_solution) < 0){
                free_all_list_nodes(best_solution);
                return -1;
            }
        }
        else{
            free_all_list_nodes(cur_solution);
        }
    }
    free_all_list_nodes(best_solution);
    return 0;
}

/**
 * Main function of the program
 * @brief This function is the entry point to the program and handles all procedures of the program.
 * @details All procedures get conducted by this function.
 * This function opens the shared memory (via a function call) and opens all semaphores. It also sets up the signal handling for SIGINT and SIGTERM.
 * After setting up everything, the best solution will be searched (via function call).
 * After finishing the process, all semaphores as well as the shared memory get closed and unlinked (via function calls).
 * @details Global variables: program_name
 * 
 * @param argc Argument counter (should be 1)
 * @param argv Argument vector (should only contain program name)
 * @return Returns EXIT_SUCCESS on successful termination of the program. Returns EXIT_FAILURE if an failure occurs at some place in the program.
 */
int main(int argc, char *argv[]){
    shm_t *shm = NULL;                              /**< Pointer that points to the shared memory*/
    sem_t *free_space = NULL;                       /**< Pointer that points to the free space semaphore*/
    sem_t *used_space = NULL;                       /**< Pointer that points to the used space semaphore*/
    sem_t *excl_access = NULL;                      /**< Pointer that points to the exclusive access semaphore*/
    struct sigaction sa;                            /**< Sigaction for handling SIGINT and SIGTERM*/
    program_name = argv[0];
    if(argc > 1){
        usage_supervisor();
        exit(EXIT_FAILURE);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    if(sigaction(SIGINT, &sa, NULL) < 0){
        error_message("sigaction");
        exit(EXIT_FAILURE);
    }
    if(sigaction(SIGTERM, &sa, NULL) < 0){
        error_message("sigaction");
        exit(EXIT_FAILURE);
    }
    shm = create_shared_memory();
    if(shm == NULL){
        exit(EXIT_FAILURE);
    }
    free_space = sem_open(SEM_FREE_SPACE, O_CREAT | O_EXCL, 0600, MAX_DATA);
    if(free_space == SEM_FAILED){
        error_message("sem_open");
        (void) cleanup_shared_memory(shm);
        exit(EXIT_FAILURE);
    }
    used_space = sem_open(SEM_USED_SPACE, O_CREAT | O_EXCL, 0600, 0);
    if(used_space == SEM_FAILED){
        error_message("sem_open");
        (void) sem_close(free_space);
        (void) sem_unlink(SEM_FREE_SPACE);
        (void) cleanup_shared_memory(shm);
        exit(EXIT_FAILURE);
    }
    excl_access = sem_open(SEM_EXCL_ACCESS, O_CREAT | O_EXCL, 0600, 1);
    if(excl_access == SEM_FAILED){
        error_message("sem_open");
        (void) sem_close(free_space);
        (void) sem_close(used_space);
        (void) sem_unlink(SEM_FREE_SPACE);
        (void) sem_unlink(SEM_USED_SPACE);
        (void) cleanup_shared_memory(shm);
        exit(EXIT_FAILURE);
    }
    int err = get_best_solution(shm, free_space, used_space);
    shm->state = -1;
    if(err < 0){
        (void) cleanup_shared_memory(shm);
        (void) close_and_unlink_all_semaphores(free_space, used_space, excl_access);
        exit(EXIT_FAILURE);
    }
    if(cleanup_shared_memory(shm) < 0){
        (void) close_and_unlink_all_semaphores(free_space, used_space, excl_access);
        exit(EXIT_FAILURE);
    }
    if(close_and_unlink_all_semaphores(free_space, used_space, excl_access) < 0){
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}