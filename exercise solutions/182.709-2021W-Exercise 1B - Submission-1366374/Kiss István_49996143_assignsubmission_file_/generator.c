/**
 * @file generator.c
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Implementation of the generator program
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
#include <time.h>
#include <unistd.h>
#include "circbuffer.h"

/**
 * @brief macro for program name
 * 
 */
#define PROG_NAME "generator"
/**
 * @brief global variable for the command the program was executed with
 * 
 */
static char *COMMAND;

/**
 * @brief global variable to signal quit using SIGINT
 * 
 */
static volatile sig_atomic_t quit = 0;

/**
 * @brief Function to handle SIGINIT
 * 
 * @param signal signal that is ignored
 */
static void handle_signal(int signal) {
    quit = 1;
}

/**
 * @brief Prints usage to stderr.
 * @details The programs terminates after printing with error.
 * 
 */
static void usage(void){
    fprintf(stderr, "Usage: %s [EDGE1...]\n", PROG_NAME);
    exit(EXIT_FAILURE) ;
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
 * @brief Mixes all input to a value for better randomness.
 * @details All variables are shifted and are changed to get a different value.
 * source: https://stackoverflow.com/questions/322938/recommended-way-to-initialize-srand
 * 
 * @param a any value
 * @param b any value
 * @param c any value
 * @return unsigned long calculated from all input
 */
unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

/**
 * @brief Handles the program arguments
 * @details It also allocates memory for the graph and creates edges for it.
 * Calls usage if no edge was specified.
 * 
 * global variables: COMMAND
 * @param argc arg counter
 * @param argv arg variables
 * @param dst pointer to graph to be created
 */
static void handle_args(int argc, char *argv[], graph* dst) {
    char c;
    dst->edges = malloc(argc * sizeof(edge));
    if (dst->edges == NULL) {
        handle_error(1, "malloc failed");
    }
    COMMAND = argv[0];
    if ((c = getopt(argc, argv, "")) != -1) {
        switch (c) {
            case '?':
                usage();
                break;
            default:
                usage();
                break;
        }
    }
    if (optind < argc) {
        char* remaining;
        edge edge;
        while (optind < argc) {
            edge.from = strtol(argv[optind], &remaining, 10);
            edge.to = strtol(remaining + 1, NULL, 10);
            optind++;
            if (add_edge(dst, &edge, 0) == NULL) {
                handle_error(1, "add_edge failed");
            }
        }
    } else {
        usage();
    }
}

/**
 * @brief Function containing the main logic of this program.
 * @details First, it creates a seed for srand() using the PID.
 * This is necessary for better randomness accross multiple generators.
 * Then creates input graph and creates buffer. Then feedback arc sets are
 *  generated and written to the buffer. It closes the buffer on error or SIGINT.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]) {
    unsigned long seed = mix(clock(), time(NULL), getpid());
    srand(seed);
    graph gr = {NULL, 0, 0};
    graph fbas = {NULL, 0, 0};
    handle_args(argc, argv, &gr);
    mybuffer circ_buffer;
    edge empty = {UINT32_MAX, UINT32_MAX};
    int max_edges = MAX_EDGES;

    struct sigaction sa = {.sa_handler = handle_signal};
    sigaction(SIGINT, &sa, NULL);
    
    if (open_buffer(&circ_buffer) < 0) {
        handle_error(1, "failed to open buffer");
    }

    while (!quit && circ_buffer.shm->healthy) {
        if ((find_feeback_arc_set(&gr, &fbas)) == NULL) {
            handle_error(1, "find_feedback_arc_set failed");
        }
        if (fbas.num_of_edges > max_edges) {
            fbas.num_of_edges = 0;
            continue;
        } else {
            max_edges = fbas.num_of_edges;
            while (fbas.num_of_edges < 8) {
                add_edge(&fbas, &empty, 1);
            }
        }
        if (write_buffer(&circ_buffer, &fbas) < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                handle_error(1, "failed to write buffer");
            }
        }
        fbas.num_of_edges = 0;
    }
    if (! circ_buffer.shm->healthy) {
        close_buffer(&circ_buffer);
    }
    free(gr.edges);
    free(fbas.edges);
    return 0;
}
