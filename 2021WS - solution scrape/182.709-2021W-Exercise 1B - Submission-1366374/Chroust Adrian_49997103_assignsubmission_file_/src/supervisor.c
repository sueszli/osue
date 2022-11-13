/**
 * @file supervisor.c
 * @author Adrian Chroust <e12023146@student.tuwien.ac.at>
 * @date 8.11.2021
 * @brief Supervisor program module.
 * @details Entrance point for a program that listens to subprocesses created from the generator module
 *          which try to find an optimal feedback arc set by returning random solutions.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>
#include <limits.h>

#include "sharedcircularbuffer.h"

/**
 * @brief The program name detected by the main function.
 * @details Required for printing out the proper usage of the program through the usage function.
 * */
static char *prog_name;

/**
 * @brief This function prints the expected input parameters of the program to stderr.
 * @details The function is usually called when a call with unexpected inputs is detected.
 */
static void usage(void) {
    fprintf(stderr, "[%s] Usage: %s\n", prog_name, prog_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Atomic signal integer indicating if a signal was detected.
 */
static volatile sig_atomic_t quit = 0;

/**
 * @brief Signal handler updating the atomic signal integer quit.
 */
static void handle_signal(int signal) {
    quit = 1;
}

/**
 * @brief Dynamically allocated memory storing a solution for feedback arc.
 * @details The data stored are edges (v1, v2) where each edge takes up two memory spaces and
 *          all edges are stored subsequently, meaning the array looks like {e1_v1, e1_v2, e2_v1, e2_v2, ...}.
 */
static unsigned int *solution_memory;

/**
 * @brief Indicates the length of the solution_memory array.
 */
static unsigned int solution_memory_length = 0;

/**
 * @brief This function sets and updates the solution memory length.
 * @details If the solution_memory has not yet been set the function uses malloc to allocate, realloc otherwise.
 *          If the given length is 0, the size is set to 1, meaning 1 byte,
 *          to prevent the solution_memory from being set to NULL.
 * @param length The new length of the solution_memory array.
 * @return -1 if the allocation or reallocation was unsuccessful, 0 otherwise.
 */
static int set_solution_memory_size(unsigned int length) {
    size_t size = length == 0 ? 1 : length * sizeof(unsigned int);
    unsigned int *new_solution_memory = solution_memory == NULL ? malloc(size) : realloc(solution_memory, size);

    if (new_solution_memory == NULL) return -1;

    solution_memory = new_solution_memory;
    solution_memory_length = length;
    return 0;
}

/**
 *  @brief The next index that should be read in the shared_buffer data array.
 */
static unsigned int buffer_read_pos = 0;

/**
 * @brief The current best solution where every edge in the solution adds a length of 2.
 * @details Starts with UINT_MAX so that every new solution is smaller than the current one.
 */
static unsigned int best_solution_length = UINT_MAX;

/**
 * @brief The next index to be written to in solution_memory.
 */
static unsigned int solution_memory_write_pos = 0;

/**
 * @brief Prints the current best solution to stdout.
 */
static void print_solution(void) {
    if (best_solution_length == 0) {
        // If the length of the solution is 0, then the graph has to be acyclic.
        printf("[%s] The graph is acyclic!", prog_name);
    } else {
        // Otherwise, all edges are printed.
        // As every edge takes up 2 spaces in the solution length, the number of edges is half that size.
        printf("[%s] Solution with %i edges:", prog_name, best_solution_length / 2);
        for (unsigned int i = 0; i < best_solution_length; i += 2) {
            printf(" %i-%i", solution_memory[i], solution_memory[i + 1]);
        }
    }
    printf("\n");
}

/**
 * @brief Reads solutions from the shared circular buffer and subsequently prints the best ones.
 * @return -1 if the semaphore access or the memory reallocation was unsuccessful, 0 otherwise.
 */
static int read_solution(void) {
    if (sem_wait(used_space_sem) != 0) return -1;

    // The next value to be read from the buffer.
    unsigned int val = (*shared_buffer).data[buffer_read_pos];

    if (sem_post(free_space_sem) != 0) return -1;

    // Set buffer_read_pos to the next index, beginning again at 0 if the last index was reached.
    buffer_read_pos = (buffer_read_pos + 1) % BUFFER_DATA_LENGTH;

    // If the best solution is already 0,
    // then the current solution does not have to be checked.
    if (best_solution_length == 0) return 0;

    // Check if the end of the current solution write was reached.
    if (val == END_OF_WRITE) {
        // Check if the current solution is better than the previous best one.
        if (solution_memory_write_pos < best_solution_length) {
            // Update and print the new best solution.
            best_solution_length = solution_memory_write_pos;
            print_solution();
            // Resize the memory to the size of the best solution as no solution
            // with more entries than the current one will be written to solution_memory.
            if (set_solution_memory_size(best_solution_length) != 0) return -1;
        }
        // Reset the solution memory write position to 0.
        solution_memory_write_pos = 0;
    } else {
        // If the best solution is still UINT_MAX and the new solution exceeds the
        // length of allocated solution_memory, then the length solution_memory has to be increased.
        if (best_solution_length == UINT_MAX && solution_memory_write_pos == solution_memory_length) {
            if (set_solution_memory_size(2 * solution_memory_length) != 0) return -1;
        }
        // If the current solution is not yet bigger than the best solution,
        // the read value can be written to the solution memory.
        if (solution_memory_write_pos < best_solution_length - 1) {
            solution_memory[solution_memory_write_pos] = val;
        }
        // Move the solution memory write position to the next index to be written.
        solution_memory_write_pos++;
    }

    return 0;
}

/**
 * @brief Program entry point.
 * @details Checks arguments, initially allocates memory and handles signals and errors.
 * @param argc The argument count of argv.
 * @param argv The arguments that the user inputted into the program. For more information please refer to the usage function.
 * @return EXIT_SUCCESS on correct usage, successful memory allocation and received interrupt signals, EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[]) {
    // Sets the program name.
    prog_name = argv[0];

    // If arguments apart from the program name were given, refer to usage.
    if (argc > 1) usage();

    // Initially sets length of solution_memory to 32.
    if (set_solution_memory_size(32) != 0) {
        fprintf(stderr, "[%s] Error: Could not allocate enough memory.\n", prog_name);
        return EXIT_FAILURE;
    }

    // Create the shared memory and semaphores.
    if (initialize_shared_circular_buffer(true) != 0) {
        free(solution_memory);
        fprintf(stderr, "[%s] Error: Could not create shared circular buffer.\n", prog_name);
        return EXIT_FAILURE;
    }

    // Listen to interrupts and termination signals.
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Continue as long as a signal is received.
    while (quit == 0) {
        if (read_solution() != 0) {
            // If an interrupt was received,
            // continue and check if quit was set to 0.
            if (errno == EINTR) continue;

            fprintf(stderr, "[%s] Error: Could not read from shared circular buffer.\n", prog_name);
            free(solution_memory);
            terminate_shared_circular_buffer(true);
            return EXIT_FAILURE;
        }
    }

    // Free all used memory.
    free(solution_memory);
    if (terminate_shared_circular_buffer(true) != 0) {
        fprintf(stderr, "[%s] Error: Could not remove shared circular buffer.\n", prog_name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
