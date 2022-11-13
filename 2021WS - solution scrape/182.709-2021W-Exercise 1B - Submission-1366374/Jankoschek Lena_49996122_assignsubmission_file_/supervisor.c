/**
 * @file supervisor.c
 * @author Lena Jankoschek - 12019852
 * @brief 
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <signal.h>

#include "circularbuffer.h"



//variable used for signal handling and termination of the while loop
volatile sig_atomic_t quit = 0;


/**
 * usage function
 * @brief this function prints the right usage of the program, its program name and exits the program with EXIT_FAILURE.
 * 
 * @param program_name - name of the program (argv[0])
 */
static void usage(const char *program_name) {
    fprintf(stderr, "[%s] Usage: %s\n", program_name, program_name);
    exit(EXIT_FAILURE);
}


/**
 * @brief this function prints the edges of a solution
 * @details this function prints the edges of the given solution. It iterates through all edges[2] and prints them in the following pattern: 1-1.
 * @param s - instance of a struct solution, which is printed
 */
static void print_edges(struct solution s) {
    //iterate through edges
    for(int i=0; i < s.size; i++) {
        fprintf(stdout, "%d-%d ", s.edges[i][0], s.edges[i][1]);
    }
    fprintf(stdout, "\n");
}


/**
 * @brief this function compares two solutions and returns the better one
 * @details this function compares the current minimal solution with the one the supervisor just read. If it is better, has a smaller number of
 * removed edges (size), than it prints that a new solution is found and returns the solution new. Otherwise the solution min is returned and nothing is prined.
 * @param min - a solution, which is the current minimal one
 * @param new - a solution, which was just read from the circularbuffer
 * @return struct solution - the better solution of the given two
 */
static struct solution compare_solutions(struct solution min, struct solution new, const char *program_name) {
    if(min.size > new.size && min.size != -1 && new.size != -1) {
        fprintf(stdout, "[%s] Solution with %d edges: ", program_name, new.size);
        print_edges(new);
        return new;
    } 
    return min;
}


/**
 * @brief this function sets the global variable quit to 1
 * @details this function is called when a SIGINT or SIGTERM signal occures. It then sets the global variable quit to 1, which ends the while loop in main.
 * @param signal - the signal number
 */
static void handle_signal(int signal) {
    quit = 1;
}



/**
 * @brief entry point of the program
 * @details this function is the entry point of the program 'supervisor'. It handles the incoming signals (SIGINT,SIGTERM), checks if the program is
 * started without arguments. It's main task is to create a circularbuffer and read the solutions that the generators create from it.
 * It saves the current best solution and prints it to stdout. If a signal SIGINT or SIGTERM occures or a solution with 0 edges is found,
 * it sets the state of the circularbuffer to 1,which then bgrins the generators to exit the program. 
 * After that the function cb_close is called, which closes the circularbuffer and frees all its resources.
 * @param argc - number of arguments of the program
 * @param argv - value of arguments of the program
 * @return int - exit code, 0 = everythings okay, != 0 = failure
 */
int main(int argc, char *argv[]) {
    //set up signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //check if number of arguments is not bigger than 1 -> no arguments
    if(argc > 1) {
        fprintf(stderr, "[%s] ERROR: Too many arguments\n", argv[0]);
        usage(argv[0]);
    }

    //open the circularbuffer
    struct circularbuffer *circbuf = cb_coo('s');
    if(circbuf == NULL) {
        fprintf(stderr, "[%s] ERROR: %s\n", argv[0], strerror(errno));
        if(errno == EEXIST) {
            fprintf(stderr, "[%s] HINT: Try 'rm -rf /dev/shm/*12019852*' for resolving the error\n", argv[0]);
        }
        exit(EXIT_FAILURE);
    }

    //minimal solution initialising
    struct solution min;
    min.size = INT_MAX;

    while(!quit) {
        //read from circularbuffer
        struct solution s = cb_read(circbuf);
        if(s.size == -1 && errno != EINTR) {
            fprintf(stderr, "[%s] ERROR: %s\n", argv[0], strerror(errno));
            //close the circularbuffer
            if(cb_close(circbuf, 's') != 0) {
                fprintf(stderr, "[%s] ERROR: %s\n", argv[0], strerror(errno));
            }
            exit(EXIT_FAILURE);
        }
        //save better solution
        min = compare_solutions(min, s, argv[0]);
        if(min.size == 0) {
            fprintf(stdout, "[%s] The graph is 3-colorable!\n", argv[0]);
            quit = true;
        }
    }

    //set state of circular buffer so all generators are closing
    circbuf->shm->state = 1;

    //close the circularbuffer
    if(cb_close(circbuf, 's') != 0) {
        fprintf(stderr, "[%s] ERROR: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}