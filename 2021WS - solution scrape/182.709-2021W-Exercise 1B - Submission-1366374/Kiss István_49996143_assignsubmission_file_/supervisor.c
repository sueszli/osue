/**
 * @file supervisor.c
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Implementation of the supervisor program.
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "graph.h"
#include "circbuffer.h"

/**
 * @details Holds program name
 * 
 */
#define PROG_NAME "supervisor"
/**
 * @details Used for saving the command that executed this program. 
 * 
 */
static char *COMMAND;
/**
 * @details Used for handling SIGINT
 * 
 */
static volatile sig_atomic_t quit = 0;

/**
 * @brief Handles signal for termination
 * @details It does not check the param.
 * 
 * @param signal required for sigaction, ignored
 */
static void handle_signal(int signal) {
    quit = 1;
}

/**
 * @brief Handles the program arguments
 * @details sets the command global variable
 * 
 * 
 * global variables: COMMAND
 * @param argc arg counter
 * @param argv arg variables
 */
static void handle_args(int argc, char *argv[]) {
    COMMAND = argv[0];
}

/**
 * @brief Handles errors
 * @details If error is fatal the program terminates after printing error to stderr.
 * 
 * 
 * global variables: COMMAND
 * @param fatal signal that error is fatal
 * @param message error message
 */
static void handle_error(int fatal, char* message) {
    if (fatal == 1) {
        fprintf(stderr, "[%s] fatal error: %s: %s \n", COMMAND, message, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "[%s] %s: %s \n", COMMAND, message, strerror(errno));
}

/**
 * @brief The main method contains the logic for this program.
 * @details Initializes signal handling and creates ring buffer.
 *  Reads all available edges from the buffer and prints the best result on stdout.
 *  The function returns if the graph is acyclic.
 * 
 * @param argc arg counter
 * @param argv argc variables
 * @return int returns EXIT_SUCCESS on termination
 */
int main(int argc, char *argv[]) {
    mybuffer circ_buffer;
    edge edges[MAX_EDGES];
    graph gr = {0};
    int min_edges = MAX_EDGES + 1;
    handle_args(argc, argv);

    struct sigaction sa = {.sa_handler = handle_signal};
    sigaction(SIGINT, &sa, NULL);

    if (create_buffer(&circ_buffer) < 0) {
        close_buffer(&circ_buffer);
        handle_error(1, "failed to create buffer");
    }

    while (!quit) {
        if (read_buffer(&circ_buffer, &edges[0]) < 0) {
            if (errno == EINTR) {
                circ_buffer.shm->healthy = 0;
                continue;
            } else {
                close_buffer(&circ_buffer);
                handle_error(1, "failed to read buffer");
            }
        }
        for (int i = 0; i < MAX_EDGES; i++) {
            if ((edges[i].from != UINT32_MAX) && (edges[i].to != UINT32_MAX)) {
                add_edge(&gr, &(edges[i]), 0);
            } else {
                break;
            }
        }
        
        if (gr.num_of_edges == 0 || (gr.num_of_edges == 1 && (gr.edges[0].from == gr.edges[0].to))) {
            printf("The graph is acyclic! \n");
            handle_signal(1);
            break;
        }
        if (gr.num_of_edges < min_edges) {
            min_edges = gr.num_of_edges;
            printf("Solution with %d edges: ", gr.num_of_edges);
            print_edges(&gr);
        }
        gr.num_of_edges = 0;
    }
    circ_buffer.shm->healthy = 0;
    close_buffer(&circ_buffer);
    free(gr.edges);
    exit(EXIT_SUCCESS);
}
