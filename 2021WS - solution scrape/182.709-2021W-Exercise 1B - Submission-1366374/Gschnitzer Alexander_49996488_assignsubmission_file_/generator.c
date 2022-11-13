/**
 * @file generator.c
 * @author Alexander Gschnitzer (01652750) <e1652750@student.tuwien.ac.at>
 * @date 21.10.2021
 *
 * @brief Generates random solutions for the 3-coloring problem.
 * @details Takes a graph as input and proposes solutions by assigning random colors to the graph. It removes edges if the adjacent vertices have the same color.
 * It writes the solution to a circular buffer which is read by supervisor.c. It repeats the procedure until it is notified by the supervisor to terminate.
 */

#include "util.h"
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <regex.h>
#include <time.h>
#include <inttypes.h>
#include <semaphore.h>
#include <errno.h>

/**
 * @brief Regular expression specifying the format of edges.
 */
#define EDGE_FORMAT "^[0-9]+-[0-9]+$"

/**
 * Graph.
 * @brief Representation of input graph.
 * @details Graph contains array of edges, the number of vertices and an array storing the color information for each vertex.
 */
typedef struct graph {
    edge_t *edges;
    vertex_t *vertices;
    int vertex_count;
} graph_t;

/**
 * @brief Global variables prog_name and arguments - used in util.c to display usage message.
 */
const char *prog_name, *arguments = "d-d [d-d [d-d [d-d ...]]] (d is an positive integer)";

/**
 * @brief Shared memory flags to define access of file - used in util.c. 0_RDWR means read and write access.
 */
const int shm_flags = O_RDWR, sem_flags = 0;

/**
 * @brief Circular buffer acting as the mapped shared memory object. Also used in util.c.
 */
cb_t *buffer;

/**
 * @brief Semaphores that store information about free and used space in circular buffer respectively.
 * sem_access is used to prevent concurrent access to circular buffer.
 */
sem_t *sem_free, *sem_used, *sem_access;

/**
 * Check format of edges.
 * @brief Checks if an edge is well-formed.
 * @details Uses a regular expression to check the correct format of the provided string. The format is defined by EDGE_FORMAT.
 * Prepares the regex by calling regcomp and executes it by using regexec. Finally, it frees all used resources.
 * @param edge string representing two connected vertices.
 * @return 0 on success, a number other than 0 if regcomp fails or REG_NOMATCH if regexec fails.
 */
static int check_edge(const char *edge) {
    int result;
    regex_t regex;

    // prepare regex
    if ((result = regcomp(&regex, EDGE_FORMAT, REG_EXTENDED)) == 0) {
        // check if edge matches regex and store it in result
        result = regexec(&regex, edge, (size_t) 0, NULL, 0);
    }

    // free used resources
    regfree(&regex);
    return result;
}

/**
 * Finds vertex in list.
 * @brief Iterates over list and returns index of found element.
 * @details Checks provided with each element and returns its index if the element was found.
 * @param id of element.
 * @param vertices array of vertices.
 * @param vertex_count number of vertices and length of vertices.
 * @return index of element if it was found, -1 otherwise.
 */
static int find_vertex(const int *id, const vertex_t *vertices, const int vertex_count) {
    for (int i = 0; i < vertex_count; ++i) {
        if (vertices[i].id == *id) {
            return i;
        }
    }

    return -1;
}

/**
 * Parse edges from input.
 * @brief Parses all positional arguments (edges) and stores them in a representation of a graph.
 * @details Parses the input, i.e. an array of edges, and checks if each of them is provided in the correct format - by calling the check_edge function.
 * If so, each edge is split into two parts, the first and second vertex, which are converted to an integer in order for them to be properly stored.
 * The vertex with the highest id is stored in vertex_count.
 * @param input array of edges provided by the positional arguments.
 * @param edges array of parsed edges.
 * @return 0 on success or -1 if any of the provided edges is malformed.
 */
static int parse_edges(char **input, edge_t *edges, vertex_t *vertices, int *vertex_count) {
    for (int i = 0, j = 0; input[i] != NULL; i++) {
        // return value is unequal to 0 since check_edge may return REG_NOMATCH
        if (check_edge(input[i]) != 0) {
            return -1;
        }

        // split input string to extract both vertices
        char *left = strtok(input[i], "-");
        char *right = strtok(NULL, "-");

        // convert vertices from char to int
        int v1 = (int) strtoumax(left, NULL, 10);
        int v2 = (int) strtoumax(right, NULL, 10);

        // create first vertex if it is not already in the list of vertices
        int v1_index = find_vertex(&v1, vertices, j + 1);
        if (v1_index == -1) {
            vertex_t vertex = {.id = v1, .color = -1};
            vertices[j++] = vertex;
        }

        // create second vertex if it is not already in the list of vertices
        int v2_index = find_vertex(&v2, vertices, j + 1);
        if (v2_index == -1) {
            vertex_t vertex = {.id = v2, .color = -1};
            vertices[j++] = vertex;
        }

        // create edge and store it in edges array
        edge_t edge = {.v1 = v1, .v2 = v2};
        edges[i] = edge;

        // updates number of vertices
        *vertex_count = j;
    }

    // increment vertex_count to reflect correct number of vertices
    *vertex_count += 1;

    return 0;
}

/**
 * Generate solutions.
 * @brief Generates solution of 3-coloring problem with given graph.
 * @details Assigns a color to each vertex, iterates over all edges and checks if the assigned colors of both vertices are equal.
 * If so, it adds the associated edge to the solution, since it needs to be removed for the graph to become a valid solution.
 * However, it discards any solution if the number of removed edges exceed the limit - set by MAX_EDGES - or all edges of the graph are removed.
 * In this case the number of removed edges is -1.
 * @param edges array of edges contained in the graph.
 * @param edge_count number of edges in the graph.
 * @return found solution, whereby removed edges is either -1 if the solutions needs to be discarded, or any other value if the solution is valid.
 */
static solution_t *generate_solution(edge_t *edges, const int edge_count, vertex_t *vertices, const int *vertex_count) {
    // generate random seed with process id, current time and elapsed clock ticks
    srand(getpid() * time(0) * clock());

    // generate random color for each vertex, rand() provides only limited randomness but suffices for this exercise
    for (int i = 0; i < *vertex_count; i++) {
        vertices[i].color = rand() % 3;
    }

    solution_t *result = (solution_t *) malloc(sizeof(solution_t));
    result->removed = 0;

    for (int i = 0; i < edge_count && result->removed < MAX_EDGES; i++) {
        int v1 = vertices[find_vertex(&edges[i].v1, vertices, *vertex_count)].color;
        int v2 = vertices[find_vertex(&edges[i].v2, vertices, *vertex_count)].color;
        if (v1 == v2) {
            result->edges[result->removed++] = edges[i];
        }
    }

    // discard solution if all edges or more than 8 edges are removed
    if (result->removed == MAX_EDGES || result->removed == edge_count) {
        result->removed = -1;
    }

    return result;
}

/**
 * Main entry point of generator.
 * @brief Generates solutions to the 3-coloring problem.
 * @details Sets program name, checks positional arguments and options. Initializes the shared memory object and semaphores,
 * reads the input and stores the provided edges in a graph. It randomly assigns colors to each vertex and removes the edges
 * with vertices that are equally colored, i.e. it generates a solution to the 3-coloring problem. Solutions are generated until
 * the supervisor terminates, either when the graph is acyclic (thus eventually a solution with 0 removed edges is found) or a signal
 * interrupts the supervisor. Finally, it frees allocated memory and exits the program.
 * @param argc Number of command-line parameters in argv.
 * @param argv Array of command-line parameters, argc elements long.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char **argv) {
    // set program name
    prog_name = argv[0];

    int c;
    while ((c = getopt(argc, argv, "\0")) != -1) {
        if (c == '?') {
            usage();
        }
    }

    // check number of positional arguments
    if ((argc - optind) < 1) {
        usage();
    }

    // open shared memory object - buffer is definitely set after the completion of this function since it exits if an error occurs
    init_buffer(0);

    // initialize free-space, used-space and access semaphores
    init_sem(0);

    // allocate memory for graph
    graph_t *graph = (graph_t *) malloc(sizeof(graph_t));
    if (graph == NULL) {
        error("Memory allocation failed during parsing of input");
    }

    // allocate memory for n edges, whereas n = argc - 1
    graph->edges = (edge_t *) malloc(sizeof(edge_t) * (argc - 1));
    if (graph->edges == NULL) {
        error("Memory allocation failed during parsing of input");
    }

    // allocate memory for vertices information with upper bound of argc - 1
    graph->vertices = (vertex_t *) malloc(sizeof(vertex_t) * argc - 1);
    if (graph->vertices == NULL) {
        error("Memory allocation failed during parsing of input");
    }

    // parse input, exit program and show usage if input is malformed
    if (parse_edges(&argv[1], graph->edges, graph->vertices, &graph->vertex_count) == -1) {
        usage();
    }

    // vertex_count must be greater than 0 since the edges are successfully parsed at this point
    assert(graph->vertex_count > 0);

    solution_t *solution;
    while (!buffer->terminate) {
        // generate solution
        solution = generate_solution(graph->edges, argc - 1, graph->vertices, &graph->vertex_count);

        // only accept solutions if they do not contain all edges nor exceed the given edge limit
        if (solution->removed != -1) {
            // blocks until circular buffer is used and not accessed by another generator
            if ((sem_wait(sem_free) == -1 || sem_wait(sem_access) == -1) && errno != EINTR) {
                error("Writing solution to queue failed");
            }

            // additional check if supervisor terminated in the meantime
            if (buffer->terminate) {
                break;
            }

            // write to buffer
            buffer->queue[buffer->w_pos] = *solution;

            // increment write position while ensuring that it stays within the bounds of the queue
            buffer->w_pos += 1;
            buffer->w_pos %= MAX_SIZE;

            // print solution to stdout for debug purposes
            print_solution(solution);

            // increment semaphore for used space and allow other generators to access buffer again
            if ((sem_post(sem_used) == -1 || sem_post(sem_access) == -1) && errno != EINTR) {
                error("Writing solution to queue failed");
            }
        }
    }

    // clean up allocated memory
    clear_buffer(0);
    clear_sem(0);
    free(solution);
    free(graph->vertices);
    free(graph->edges);
    free(graph);

    exit(EXIT_SUCCESS);
}
