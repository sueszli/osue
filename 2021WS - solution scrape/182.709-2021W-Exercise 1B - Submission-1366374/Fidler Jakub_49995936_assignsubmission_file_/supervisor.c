/**
 * @file supervisor.c
 * @author Jakub Fidler 12022512
 * @date 13 Nov 2021
 * @brief Reads solutions from a buffer and prints the best so far. Terminates when graph is 3-colorable.
 * @details Initiates setup of buffer, reads solutions from it and prints the best so far.
 * Terminates when the graph is 3-colorable. Cleans up the buffer on exit.
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include "buffer.h"

const char *PROGRAM_NAME;

/**
 * @brief Prints edges of a solution to the 3-coloring problem.
 * @details Prints edges of a solution to the 3-coloring problem. Stops on reading a node
 * value of -1.
 * @param edges are edges that will be printed
 */
void print_edges(int edges[SOLUTION_MAX_NUM_EDGES][2]);

/**
 * @brief Prints an error message to stderr and exits.
 * @details Prints an error message to stderr and exits it. If errno is set, the reason will also be printed.
 * Exits the program with EXIT_FAILURE.
 * @param error_message will be printed to stderr
 */
static void error(char *error_message);

/**
 * @brief Prints a message about the correct usage of the program.
 * @details Prints a message about the correct usage of the program in the format Usage... Example...
 */
static void usage();

/**
 * @brief Cleans up resources and then shows error.
 * @details Tries cleaning up resources, if this fails shows one error.
 * Else shows the error given by the error_message argument.
 * Calls error in any case.
 * @param error_message will be printed to stderr
 */
static void clean_and_show_error(char *error_message);


/**
 * @brief Entrypoint of the program.
 * @param argc amount of arguments
 * @param argv array of arguments to the program. Should be empty except for index 0. Which
 * contains the filepath.
 */
int main(int argc, char **argv)
{
    if (argc != 1)
        usage();

    PROGRAM_NAME = argv[0];

    if (!buffer_setup())
        clean_and_show_error("buffer setup failed");

    int min_solution_size = -1;
    int edges[SOLUTION_MAX_NUM_EDGES][2];
    while (!buffer_has_terminated())
    {

        if (!buffer_read(edges))
            clean_and_show_error("failed reading from buffer");

        // determine amount of edges in solution
        int num_edges = 0;
        while (num_edges != SOLUTION_MAX_NUM_EDGES)
        {
            int a = edges[num_edges][0];
            // -1 as a node value signals that the previous edge was the last one
            if (a == -1)
            {
                break;
            }
            num_edges++;
        }

        if (num_edges == 0)
        {
            printf("The graph is 3-colorable!\n");
            break;
        }

        else if (num_edges < min_solution_size || min_solution_size == -1)
        {
            min_solution_size = num_edges;
            printf("Solution with %d edge(s): ", num_edges);
            print_edges(edges);
        }
    }

    buffer_terminate();

    if (!buffer_close())
        clean_and_show_error("closing buffer failed");

    if (!buffer_clean_up())
        error("buffer clean-up failed");
}

void print_edges(int edges[SOLUTION_MAX_NUM_EDGES][2])
{
    for (int i = 0; i < SOLUTION_MAX_NUM_EDGES; i++)
    {
        int a = edges[i][0];
        int b = edges[i][1];
        // -1 as a node value signals that the previous edge was the last one
        if (a == -1)
            break;
        printf("%d-%d   ", a, b);
    }
    puts("");
}

static void error(char *error_message)
{
    fprintf(stderr, "[%s] ERROR: %s: %s.\n", PROGRAM_NAME, error_message, strcmp(strerror(errno), "Success") == 0 ? "Failure" : strerror(errno));
    exit(EXIT_FAILURE);
}

static void usage(void)
{
    fprintf(stderr, "Usage: %s}\n", PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

static void clean_and_show_error(char *error_message)
{
    if (!buffer_clean_up())
        error("buffer clean-up failed while exiting due to other error");

    error(error_message);
}