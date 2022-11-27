#include "circular_buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/**
 * @file generator.c
 * @author Elias GALL - 12019857
 * @brief implements a generator approximating the 'minimum feedback arc set' problem
 * @details Implements a generator approximating the 'minimum feedback arc set' problem using the
 *          algorithm supplied with the exercise. Writes the results to a circular buffer, which
 *          is managed by another process.
 * @date 2021-11-02
 */

/** @brief solution to 'minimum feedback arc set' currently being worked on */
static buffer_entry_t solution;

/** @brief list of all edges in the graph */
static edge_t *edges = NULL;

/** @brief number of edges in the graph */
static int edges_count = 0;

/** @brief list of vertices in the graph */
static int *vertices = NULL;

/** @brief size of the list of vertices */
static int vertices_size = 32;

/** @brief number of vertices in the graph */
static int vertex_count = 0;

/**
 * @brief parses provided arguments
 * @details Parses and validates arguments by checking their count and parsing them
 *          individually using 'parse_single_argument'.
 * @details Global variables used: edges, edges_count, vertices, vertices_size
 * @param argc argument counter
 * @param argv argument vector
 * @return 0 ... success
 * @return -1 ... error
 */
static int parse_arguments(int argc, char *argv[]);

/**
 * @brief checks whether a character is a digit or a dash or not
 * @details Checks if the provided character is a decimal digit or a dash
 * @param symbol digit to check
 * @return 0 ... is digit
 * @return -1 ... is no digit
 */
static int is_digit(char symbol);

/**
 * @brief checks if a single argument adheres to the format
 * @details Validates an argument by checking if all characters are allowed
 *          and counting digits before and after the single dash that is allowed
 *          and has to occur.
 * @param arg argument to check
 * @return 0 ... argument valid
 * @return -1 ... argument invalid
 */
static int validate_argument(char *arg);

/** 
 * @brief parses a single argument after validating it
 * @details Parses an argument by converting the two parts seperated by the dash to int and
 *          storing the edge in 'edges'. Also calls 'add_vertex_if_not_exists' to track all
 *          vertices of the graph.
 * @details Global variables used: edges
 * @param arg the argument to parse
 * @param edge_index the index of the new edge in 'edges'
 * @param program_name program name to include in error messages
 * @return 0 ... success
 * @return -1 ... failure
 */
static int parse_single_argument(char *arg, int edge_index, char *program_name);

/**
 * @brief adds vertex to 'vertrices' if it is not contained already
 * @details Adds a vertex to 'vertices' unless it is already part of the list. Dynamically increases
 *          the size of the list.
 * @details Global variables used: vertices, vertex_count, vertices_size
 * @param vertex vertex to add
 * @param program_name program name included in error messages
 * @return 0 ... success (does not indicate whether the vertex was added or not)
 * @return -1 ... failure
 */
static int add_vertex_if_not_exists(int vertex, char *program_name);

/**
 * @brief shuffles 'vertices' randomly
 * @details Shuffles 'vertices' using the Fisher-Yates algorithm back to front.
 * @details Global variables used: vertex_count, vertices
 * @param program_name program name included in error messages
 * @return 0 ... success
 * @return -1 ... failure
 */
static int shuffle_vertices(char *program_name);

/**
 * @brief generates a random integer
 * @details Generates a random integer between 0 and (excluding) 'max' using 'gettimeofday'
 *          and 'timeval.tv_usec'.
 * @param max upper bound, not included
 * @param program_name program name included in error messages
 */
static int get_random_int(int max, char *program_name);

/**
 * @brief checks if edge is part of the current solution
 * @details Checks if the destination of an edge appears before it's source in the current permutation
 *          of 'vertices'.
 * @details Global variables used: vertex_count, vertices
 * @param edge edge to check
 * @return 0 ... not include
 * @return 1 ... include
 */
static int include_edge(edge_t edge);

/**
 * @brief extracts solution from current permutation of vertices
 * @details Extracts solution from permutation by checking every element in 'edges' if it should
 *          be included in a solution. If a solution is too long, it is discarded.
 * @details Global variables used: solution, edges, edges_count
 * @return 0 ... solution is valid
 * @return -1 ... solution too long
 */
static int extract_solution(void);

/**
 * @brief implements the main loop for the generator
 * @details Connects to the circular buffer. In the main loop 'vertices' is shuffled and a solution
 *          extracted. If it is valid, it is written to the buffer. After 'terminate' is set in the buffer
 *          disconnects from the circular buffer and exits.
 * @details Global variables used: solution
 * @param argc argument counter
 * @param argv argument vector
 * @return EXIT_SUCCESS ... success
 * @return EXIT_FAILURE ... failure
 */
int main(int argc, char *argv[]) {
    int error = 0;
    if (parse_arguments(argc, argv) == -1) {
        error++;
        return EXIT_FAILURE;
    } 
    if (error == 0) {
        do {
            if (connect(BUFFER_NAME) == -1) {
                error++;
                continue;
            }
            while (1) {
                shuffle_vertices(argv[0]);
                if (extract_solution() != -1) {
                    if (write_buffer(&solution, argv[0]) == -1) {
                        if (get_terminate() == 1) {
                            // terminate
                            printf("terminating\n");
                        } else {
                            // error
                            error++;
                        }
                        break;
                    }
                }
            }
        } while (0);
    }
    disconnect(argv[0]);
    if (vertices != NULL) {
        free(vertices);
    }
    if (edges != NULL) {
        free(edges);
    }
    return error == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int shuffle_vertices(char *program_name) {
    int i = 0;
    int r = 0;
    int swap = 0;
    for (i = vertex_count - 1; i > 0; i--) {
        r = get_random_int(i, program_name);
        if (r == -1) {
            return -1;
        }
        swap = vertices[r];
        vertices[r] = vertices[i];
        vertices[i] = swap;
    }
    return 0;
}

static int extract_solution(void) {
    edge_t solution_edges[MAX_RESULT_EDGES];
    memset(solution_edges, 0, sizeof(solution_edges));
    int solution_edge_count = 0;
    int i = 0;
    for (i = 0; i < edges_count; i++) {
        if (include_edge(edges[i]) == 1) {
            if (solution_edge_count == MAX_RESULT_EDGES) {
                // too many edges in result
                return -1;
            }
            memcpy(&solution_edges[solution_edge_count], &edges[i], sizeof(edge_t));
            solution_edge_count++;
        }
    }
    memcpy(solution.edges, solution_edges, sizeof(solution_edges));
    solution.number_of_edges = solution_edge_count;
    return 0;
}

static int include_edge(edge_t edge) {
    int i = 0;
    int to_found = 0;
    for (i = 0; i < vertex_count; i++) {
        if (edge.to == vertices[i]) {
            to_found = 1;
        }
        if (to_found == 1 && vertices[i] == edge.from) {
            return 1;
        }
    }
    return 0;
}

static int get_random_int(int max, char *program_name) {
    struct timeval t;
    int i = time(NULL);
    if (gettimeofday(&t, NULL) == -1) {
        fprintf(stderr, "%s: gettimeofday failed - %s", program_name, strerror(errno));
        return -1;
    }
    int j = t.tv_usec;
    return (i ^ j) % max;
}

static int parse_arguments(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "SYNOPSIS\n\tgenerator EDGE1...\nEXAMPLE\n\tgenerator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0\n");
        return -1;
    }
    edges_count = argc - 1;
    edges = malloc(sizeof(edge_t) * (edges_count));
    if (edges == NULL) {
        fprintf(stderr, "%s: malloc failed - %s", argv[0], strerror(errno));
        return -1;
    }
    int i;
    vertices = malloc(sizeof(int) * vertices_size);
    if (vertices == NULL) {
        fprintf(stderr, "%s: malloc failed - %s", argv[0], strerror(errno));
        return -1;
    }
    for (i = 1; i < argc; i++) {
        if (parse_single_argument(argv[i], i - 1, argv[0]) == -1) {
            fprintf(stderr, "SYNOPSIS\n\tgenerator EDGE1...\nEXAMPLE\n\tgenerator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0\n");
            return -1;
        }
    }
    return 0;
}

static int parse_single_argument(char *arg, int edge_index, char *program_name) {
    if (validate_argument(arg) == -1) {
        return -1;
    }
    char *delim = index(arg, (int)'-');
    int from_vertex = strtol(arg, NULL, 10);
    int to_vertex = strtol(delim + sizeof(char), NULL, 10);
    add_vertex_if_not_exists(from_vertex, program_name);
    add_vertex_if_not_exists(to_vertex, program_name);
    edges[edge_index].from = from_vertex;
    edges[edge_index].to = to_vertex;
    
    return 0;
}

static int add_vertex_if_not_exists(int vertex, char *program_name) {
    int i = 0;
    for (i = 0; i < vertex_count; i++) {
        if (vertices[i] == vertex) {
            // exists
            return 0;
        }
    }
    vertices[vertex_count] = vertex;
    vertex_count++;
    if (vertex_count == vertices_size) {
        vertices = realloc(vertices, sizeof(vertices) * 2);
        if (vertices == NULL) {
            // error
            fprintf(stderr, "%s: realloc failed - %s", program_name, strerror(errno));
            return -1;
        }
        vertices_size *= 2;
    }
    return 0;
}

static int validate_argument(char *arg) {
    int before_dash = 0;
    int dash = 0;
    int after_dash = 0;
    int len = strlen(arg);
    int i = 0;
    for (i = 0; i < len; i++) {
        if (is_digit(arg[i]) == -1) {
            return -1;
        }
        if (arg[i] != '-') {
            if (dash == 0) {
                before_dash++;
            } else {
                after_dash++;
            }
        } else {
            dash++;
            if (dash > 1) {
                return -1;
            }
        }
    }
    return before_dash > 0 && after_dash > 0 ? 0 : -1;
}

static int is_digit(char symbol) {
    return (symbol < '0' || symbol > '9') && symbol != '-' ? -1 : 0;
}