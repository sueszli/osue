/**
 * @file generator.c
 * @author Jakub Fidler 12022512
 * @date 13 Nov 2021
 * @brief Finds solutions to the 3-coloring problem and writes them to a buffer.
 * @details Finds random solutions to the 3-coloring problem on the graph that is given
 * as arguments from the command line. Writes solutions to the buffer.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "buffer.h"

const char *PROGRAM_NAME;

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
 * @brief Generates a random solution to the 3-coloring problem and writes it to the buffer.
 * @details Generates a random solution to the 3-coloring problem for the given graph
 * and writes it to the buffer. If the solution is shorter than SOLUTION_MAX_NUM_EDGES
 * then -1 for node values signal that the previous edge was the last one.
 * @param edges pointer to 2d array of edges of the graph
 * @param num_nodes amount of nodes in the graph
 * @param num_edges amount of edges in the graph
 */
void generate_solution_and_write_to_buffer(int **edges, int num_nodes, int num_edges);

/**
 * @brief Entrypoint of the program.
 * @param argc amount of arguments
 * @param argv array of arguments to the program. all except the first should 
 * match the regex "\d+-\d+" and represent edges of the input graph
 */
int main(int argc, char **argv)
{
    PROGRAM_NAME = argv[0];

    if (argc == 1)
        usage();
    if (getopt(argc, argv, "") != -1)
        usage();

    srand(time(0) * getpid()); // sets random seed for coloring the graph

    // parse input edges
    // ... allocate 2d array for edges
    int size = sizeof(int *) * argc + sizeof(int) * argc * 2;
    int **edges = malloc(size);
    memset(edges, 0, size);
    int *ptr = (int *)(edges + argc);
    // ... point pointers to their respective arrays
    for (int i = 0; i < argc; i++)
    {
        edges[i] = ptr + (2 * i);
    }
    // ... parse edge data
    for (int i = 1; i < argc; i++)
    {
        char delimeters[] = "-";
        char *split_ptr = strtok(argv[i], delimeters);
        // no "-" in argument
        if (strtok(NULL, delimeters) == NULL)
            usage();
        // more than one "-" in argument
        if (strtok(NULL, delimeters) != NULL)
            usage();

        char *end_ptr = (argv[i] + strlen(argv[i]));

        int a = (int)strtol(argv[i], &split_ptr, 10);
        edges[i - 1][0] = a;
        int b = (int)strtol((split_ptr + 1), &end_ptr, 10);
        edges[i - 1][1] = b;
    }

    // determine amount of nodes
    // by determining the largest node number in an edge
    int num_nodes = 1;
    for (int i = 0; i < argc; i++)
    {
        if (edges[i][0] == edges[i][1])
            break;
        if (edges[i][0] + 1 > num_nodes)
            num_nodes = edges[i][0] + 1;
        if (edges[i][1] + 1 > num_nodes)
            num_nodes = edges[i][1] + 1;
    }

    int num_edges = argc - 1;

    if (!buffer_open())
        error("opening buffer failed");

    while (!buffer_has_terminated())
    {
        generate_solution_and_write_to_buffer(edges, num_nodes, num_edges);
    }

    if (!buffer_close())
        error("closing buffer failed");

    free(edges);

    return 0;
}

void generate_solution_and_write_to_buffer(int **edges, int num_nodes, int num_edges)
{
    // assign random number from 1 to 3 to each node
    int node_colors[num_nodes];
    for (int i = 0; i < num_nodes; i++)
    {
        node_colors[i] = rand() % 3;
    }

    int result[SOLUTION_MAX_NUM_EDGES][2];
    // for each node check if the adjacient nodes have the same color, if yes add them to the result
    int result_idx = 0;
    for (int i = 0; i < num_edges; i++)
    {
        int node_a = edges[i][0];
        int node_b = edges[i][1];
        if (node_colors[node_a] == node_colors[node_b])
        {
            if (result_idx == SOLUTION_MAX_NUM_EDGES)
                break;
            result[result_idx][0] = node_a;
            result[result_idx][1] = node_b;
            result_idx++;
        }
    }
    // ... -1 as a node value signals that the previous edge was the last one
    if (result_idx != SOLUTION_MAX_NUM_EDGES)
    {
        result[result_idx][0] = -1;
        result[result_idx][1] = -1;
    }

    if (!buffer_write(result) && !buffer_has_terminated())
        error("failed writing to buffer");
}

static void error(char *error_message)
{
    fprintf(stderr, "[%s] ERROR: %s: %s.\n", PROGRAM_NAME, error_message, strcmp(strerror(errno), "Success") == 0 ? "Failure" : strerror(errno));
    exit(EXIT_FAILURE);
}

static void usage(void)
{
    fprintf(stderr, "Usage: %s EDGE1...}\nExample: %s 0-1 0-2 0-3 1-2 1-3 2-3\n", PROGRAM_NAME, PROGRAM_NAME);
    exit(EXIT_FAILURE);
}