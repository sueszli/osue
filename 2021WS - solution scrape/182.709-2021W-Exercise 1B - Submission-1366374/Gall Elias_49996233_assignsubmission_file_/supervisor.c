#include "circular_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

/**
 * @file supervisor.c
 * @author Elias GALL - 12019857
 * @brief Implements a supervisor for multiple generator processes, sets up and manages a shared circular buffer.
 * @details Implements a supervisor using a circular buffer to read 'minimum feedback arc set' results computed by
 *          multiple generators. The supervisor manages the buffer by creating and removing all associated resources.
 *          It also handles SIGINT and SIGTERM by setting the 'terminate' flag in the buffer.
 * @date 2021-11-02
 */

/**
 * @brief parses arguments given by the user
 * @details Accepts if there is exacly one argument: the name of the program.
 * @param argc ... argument counter
 * @param argv ... argument vector
 * @return 0 ... success
 * @return -1 ... failure
 */
static int parse_args(int argc, char *argv[]);

/**
 * @brief handles SIGINT and SIGTERM
 * @details Sets the 'terminate' flag of the buffer to 1.
 * @param signal code of the signal triggering the function call
 * @return void
 */
static void handle_signal(int signal);

/**
 * @brief sets up signal handling for SIGINT and SIGTERM
 * @details Creates sigactions to handle SIGINT and SIGTERM and assigns handle_signal as handler.
 * @return void
 */
static void setup_signals(void);

/**
 * @brief prints a solution to stdout
 * @details Iterates over all edges in the solution provided and prints them to stdout.
 * @param solution pointer to a solution found by a generator
 * @return void
 */
static void print_solution(buffer_entry_t *solution);

/** @brief pointer to the best solution read by the supervisor so far */
static buffer_entry_t *best_solution = NULL;

/**
 * @brief entry point for the supervisor
 * @details Implements the main loop, reading solutions from the buffer and printing them to stdout,
 *          as well as saving them, if they are better than the best previously known. Terminates if
 *          'terminate' is set in the buffer, or if the graph is acyclic. Creates and cleans up all
 *          resources of the circular buffer using functions of the buffer.
 * @details Global variables used: best_solution
 * @param argc argument counter
 * @param argv argument vector
 * @return EXIT_SUCCESS ... success
 * @return EXIT_FAILURE ... error occured
 */
int main(int argc, char *argv[]) {
    if (parse_args(argc, argv) == -1) {
        return EXIT_FAILURE;
    }
    if (create(BUFFER_NAME) == -1) {
        close_after_create(BUFFER_NAME);
        return EXIT_FAILURE;
    }

    setup_signals();
    buffer_entry_t *entry = NULL;
    while (get_terminate() == 0) {
        entry = read_buffer(argv[0]);
        if (entry == NULL) {
            // error or interrupt
            // terminate
            break;
        }
        if (entry->number_of_edges == 0) {
            printf("The graph is acyclic!\n");
            free(entry);
            entry = NULL;
            break;
        }
        if (best_solution == NULL || best_solution->number_of_edges > entry->number_of_edges) {
            // new solution is better
            free(best_solution);
            best_solution = entry;
            print_solution(entry);
        }
        else {
            free(entry);
            entry = NULL;
        }
    }
    if (entry != NULL) free(entry);
    if (best_solution != NULL) free(best_solution);
    close_after_create(BUFFER_NAME);
    return EXIT_SUCCESS;
}

static void print_solution(buffer_entry_t *solution) {
    int i = 0;
    printf("Solution with %i edges:", solution->number_of_edges);
    for (i = 0; i < solution->number_of_edges; i++) {
        printf(" %i - %i", solution->edges[i].from, solution->edges[i].to);
    }
    printf("\n");
}

static void setup_signals(void) {
    // SIGINT
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_signal;
    sigaction(SIGINT, &sa_int, NULL);

    // SIGTERM
    struct sigaction sa_term;
    memset(&sa_term, 0, sizeof(sa_term));
    sa_term.sa_handler = handle_signal;
    sigaction(SIGTERM, &sa_term, NULL);
}

static void handle_signal(int signal) {
    set_terminate(1);
}

static int parse_args(int argc, char *argv[]) {
    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return -1;
    } 
    return 0;
}