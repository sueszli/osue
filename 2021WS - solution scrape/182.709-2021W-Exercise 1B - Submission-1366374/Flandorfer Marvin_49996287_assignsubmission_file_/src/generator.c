/**
 * Generator module
 * @file generator.c
 * @author Marvin Flandorfer, 52004069
 * @date 30.10.2021
 * 
 * @brief This is the generator module.
 * This is the main entry point for the generator program and this program should only be started after the supervisor program.
 * 
 * @details This module is responsible for creating solutions and writing them into the circular buffer.
 * This module also opens all semaphores (that were created by the supervisor program) and closes them after the signal for termination.
 * The program terminates when the state of the circular buffer is no longer set to 0 (this happens after the supervisor program started to terminate).
 */

#include <getopt.h>
#include <limits.h>
#include "semaphores.h"
#include "shmgen.h"
#include "list.h"
#include "algorithm.h"
#include "misc.h"

char* program_name;                                 /**< Program name*/

/**
 * Function for writing into the buffer
 * @brief This function writes a value into the circular buffer.
 * @details This function writes the given integer into the given shared memory object.
 * It only start writing after waiting for free space semaphore and posts the semaphore used space after writing.
 * 
 * @param shm Pointer to the shared memory object
 * @param free Pointer to the free space semaphore
 * @param used Pointer to the used space semaphore
 * @param val Integer that will be written into the circular buffer
 * @return Returns 0 on success (or when program is going to terminate). Returns -1 on failure due to an error.
 */
static int write_into_buffer(shm_t *shm, sem_t *free, sem_t *used, int val){
    if(shm->state == 0){
        if(sem_wait(free) < 0){
            error_message("sem_wait");
            return -1;
        }
        shm->buffer[shm->write_pos] = val;
        if(sem_post(used) < 0){
            error_message("sem_post");
            return -1;
        }
        shm->write_pos++;
        shm->write_pos %= sizeof(shm->buffer)/sizeof(int);
        return 0;
    }
    return 0;
}

/**
 * Function for closing all semaphores
 * @brief This function closes all semaphores.
 * @details All semaphores will be closed one after the other.
 * 
 * @param free Pointer to the free space semaphore
 * @param used Pointer to the used space semaphore
 * @param access Pointer to the exclusive access semaphore
 * @return Returns 0 on success. Returns -1 on failure.
 */
static int close_all_semaphores(sem_t *free, sem_t *used, sem_t *access){
    if(sem_close(free) < 0){
        error_message("sem_close");
        (void) sem_close(used);
        (void) sem_close(access);
        return -1;
    }
    if(sem_close(used) < 0){
        error_message("sem_close");
        (void) sem_close(access);
        return -1;
    }
    if(sem_close(access) < 0){
        error_message("sem_close");
        return -1;
    }
    return 0;
}

/**
 * Function for writing an entire feedback arc set to the circular buffer
 * @brief This function writes a feedback arc set to the circular buffer.
 * @details This function only starts working when the exclusive access semaphore is unlocked.
 * After that, each value of the feedback arc set will be written into the buffer.
 * After reaching the end of the solution, the function ends and returns.
 * 
 * @param shm Pointer to the shared memory object
 * @param free Pointer to the free space semaphore
 * @param used Pointer to the used space semaphore
 * @param access Pointer to the exclusive access semaphore
 * @param fb_arc_set Pointer to the head of the feedback arc set list
 * @return Returns 0 on success. Returns -1 on failure (due to an error).
 */
static int write_fb_arc_set_to_buffer(shm_t *shm, sem_t *free, sem_t *used, sem_t *access, edge_t *fb_arc_set){
    edge_t *cur = fb_arc_set;                                           /**< Current node in the list*/
    if(sem_wait(access) < 0){
        error_message("sem_wait");
        return -1;
    }
    while(cur != NULL){
        if(shm->state == 0){
            int a = write_into_buffer(shm, free, used, cur->from);
            if(a < 0){
                (void) sem_post(access);
                return -1;
            }
            if(shm->state != 0){
                if(sem_post(access) < 0){
                    error_message("sem_post");
                    return -1;
                }
                return 0;
            }
            a = write_into_buffer(shm, free, used, cur->to);
            if(a < 0){
                (void) sem_post(access);
                return -1;
            }
            cur = cur->next;
        }
        else{
            if(sem_post(access) < 0){
                error_message("sem_post");
                return -1;
            }
            return 0;
        }
    }
    if(write_into_buffer(shm, free, used, -2) < 0){
        (void) sem_post(access);
        return -1;
    }
    if(sem_post(access) < 0){
        error_message("sem_post");
        return -1;
    }
    return 0;
}

/**
 * Main function for the generator program
 * @brief This is the main function for generators.
 * @details This is the entry point for each generator program. This function conducts all processes of the generator program.
 * In addition, it also checks the program call, opens semaphores, etc.
 * At first the generator will be set up. After that it continously looks for solutions and only stops after the supervisor notifies termination.
 * At the end, all semaphores will be closed again and the shared memory will be cleaned up.
 * 
 * @param argc Argument counter
 * @param argv Argument vector
 * @return Returns EXIT_SUCCESS on successful termination. Returns EXIT_FAILURE if an error occurs somewhere in the program.
 */
int main(int argc, char *argv[]){
    int edge_count;                             /**< Number of edges*/
    int node_count;                             /**< Number of nodes*/
    int c;
    edge_t *head = NULL;                        /**< Head of the graph edge list*/
    edge_t *fb_arc_set_head = NULL;             /**< Head of the feedback arc set list*/
    shm_t *shm = NULL;                          /**< Pointer to the shared memory object*/
    sem_t *free_space = NULL;                   /**< Pointer to the free space semaphore*/
    sem_t *used_space = NULL;                   /**< Pointer to the used space semaphore*/
    sem_t *excl_access = NULL;                  /**< Pointer to the exclusive access semaphore*/
    program_name = argv[0];
    if((c = getopt(argc, argv, "")) != -1){
        usage_generator();
        exit(EXIT_FAILURE);
    }
    edge_count = argc - 1;
    if(edge_count < 1){
        usage_generator();
        exit(EXIT_FAILURE);
    }
    for(int i = 1; i < argc; i++){
        char *before = argv[i];
        char *after = NULL;
        long nr = strtol(before, &after, 10);
        if(nr == LONG_MIN || nr == LONG_MAX){
            error_message("strtol");
            exit(EXIT_FAILURE);
        }
        if(nr == 0 && strcmp(before,after) == 0){
            usage_generator();
            exit(EXIT_FAILURE);
        }
        before = after;
        char c = before[0];
        if(c != '-'){
            usage_generator();
            exit(EXIT_FAILURE);
        }
        before++;
        nr = strtol(before, &after, 10);
        if(nr == LONG_MIN || nr == LONG_MAX){
            error_message("strtol");
            exit(EXIT_FAILURE);
        }
        if(nr == 0 && strcmp(before,after) == 0){
            usage_generator();
            exit(EXIT_FAILURE);
        }
    }
    head = create_generator_edge_list(argv, edge_count);
    if(head == NULL){
        exit(EXIT_FAILURE);
    }
    node_count = get_number_of_nodes(head);
    int nodes[node_count+1];
    for(int i = 0; i <= node_count; i++){
        nodes[i] = i;
    }
    shm = open_shared_memory();
    if(shm == NULL){
        free_all_list_nodes(head);
        exit(EXIT_FAILURE);
    }
    free_space = sem_open(SEM_FREE_SPACE, 0);
    if(free_space == SEM_FAILED){
        error_message("sem_open");
        free_all_list_nodes(head);
        (void) cleanup_shared_memory(shm);
        exit(EXIT_FAILURE);
    }
    used_space = sem_open(SEM_USED_SPACE, 0);
    if(used_space == SEM_FAILED){
        error_message("sem_open");
        free_all_list_nodes(head);
        (void) sem_close(free_space);
        (void) cleanup_shared_memory(shm);
        exit(EXIT_FAILURE);
    }
    excl_access = sem_open(SEM_EXCL_ACCESS, 0);
    if(excl_access == SEM_FAILED){
        error_message("sem_open");
        free_all_list_nodes(head);
        (void) sem_close(free_space);
        (void) sem_close(used_space);
        (void) cleanup_shared_memory(shm);
        exit(EXIT_FAILURE);
    }

    while(shm->state == 0){
        fb_arc_set_head = get_feedback_arc_set(head, nodes, node_count+1);
        if(fb_arc_set_head == NULL){
            free_all_list_nodes(head);
            (void) cleanup_shared_memory(shm);
            (void) close_all_semaphores(free_space, used_space, excl_access);
            exit(EXIT_FAILURE);
        }
        if(fb_arc_set_head->from == -2){
            free_all_list_nodes(fb_arc_set_head);
            continue;
        }
        int a = write_fb_arc_set_to_buffer(shm, free_space, used_space, excl_access, fb_arc_set_head);
        if(a < 0){
            free_all_list_nodes(head);
            free_all_list_nodes(fb_arc_set_head);
            (void) cleanup_shared_memory(shm);
            (void) close_all_semaphores(free_space, used_space, excl_access);
            exit(EXIT_FAILURE);
        }
        free_all_list_nodes(fb_arc_set_head);
    }

    free_all_list_nodes(head);
    if(cleanup_shared_memory(shm) < 0){
        (void) close_all_semaphores(free_space, used_space, excl_access);
        exit(EXIT_FAILURE);
    }
    if(close_all_semaphores(free_space, used_space, excl_access) < 0){
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}