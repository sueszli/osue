/**
 * @file supervisor.c
 * @author Anni Chen
 * @date 09.11.2021
 * @brief Module to read and output solutions
 * @details Supervises the generator processes, reads the solutions from the circular buffer and outputs them
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "circular_buffer.h"


#define USAGE()                                           \
    {                                                     \
        fprintf(stdout, "[USAGE]: ./%s \n", name); \
        exit(EXIT_FAILURE);                               \
    }


#define ERROR_MSG(...)                        \
    {                                         \
        fprintf(stderr, "[%s] [ERROR]: ", name); \
        fprintf(stderr, __VA_ARGS__);         \
        fprintf(stderr, "\n");                \
    }

#define ERROR_EXIT(...)         \
    {                           \
        ERROR_MSG(__VA_ARGS__); \
        exit(EXIT_FAILURE);     \
    }

/**
 * Name of the current program.
 */
static char *name = "supervisor";

/**
 * @brief indicates whether the process should terminate
 */
volatile sig_atomic_t quit = 0;

/**
 * Signal handler
 * @brief This function sets the global quit variable to 1.
 * @param signal the signal number
 */
static void handle_signal(int signal)
{
	quit = 1;
}


/**
 * @brief The entry point of the supervisor program.
 * @details This function sets up the circular buffer and continuously reads entries from the buffer that have not been read.
 * It compares the current solution read to the previous best solution and only outputs the best one once. If a solution indicates
 * that the graph is 3-colorable, the supervisor and all the generator processes will be terminated.
 * @param argc The argument counter.
 * @param argv The argument values.
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (argc > 1)
    {
        USAGE();
    }

    //open circular buffer
    if (open_circbuf('s') == -1)
    {
        ERROR_EXIT("Failed to open circular buffer");
    }


    // initialize semaphores
    if (init_sem('s') == -1)
    {
        ERROR_EXIT("Failed to initialise semaphores");
    }

    // initialize with the highest integer value to get the minimum of the edges
    int bestSolution = INT_MAX;

    while (!quit)
    {

        struct Solution sol = read_solution();
        if (sol.status == -1)
        {
            if (errno == EINTR) // signal
                break;
            ERROR_EXIT("Failed to read from circular buffer");
        }

    // optimal solution was found
        if (sol.numOfRemovedEdges == 0)
        {
            printSolution(sol);
            quit = 1;
            break;
        }

    // a better solution was found
        if (sol.numOfRemovedEdges < bestSolution)
        {
            bestSolution = sol.numOfRemovedEdges;
            printSolution(sol);

        }
    }


    // close buffer
    if (close_circbuf('s') == -1)
    {
        ERROR_EXIT("Failed to close buffer");
    }

    return EXIT_SUCCESS;
}