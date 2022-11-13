/**
 * @file main.c
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 09.11.2021
 *
 * @brief Generator program which generates random solutions for random color schemes.
 *
 * @details Searches for solutions which are less then MIN_BOUNDARY and reports them to the supervisor programm
 * via shared memory.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <zconf.h>
#include <sys/time.h>
#include "graph.h"
#include "circular_buffer.h"

#define MIN_BOUNDARY (8)

/**
 * @brief Splits a string at a given character.
 * @details Just a wrapper method so the error handling isn't written twice.
 *
 * @param str String to be splitted or NULL if you want the next index of the string you just splitted.
 * @return Returns splitted string
 */
static char *split_str(char *str) {
    char *node = strtok(str, "-");
    if (node == NULL) {
        fprintf(stderr,
                "[./generator] Error: Malformed string arguments. \nUsage: ./generator [NODE_ID-NODE_ID] ... \n");
        return NULL;
    }
    return node;
}

/**
 * @brief Splits a string like 5-4 at the '-' symbol and converts the chars to numbers.
 * @param str The string to be parsed.
 * @param arr Array to be filled with the two parsed integers.
 */
static bool parse_numbers(char *str, long *arr) {
    char *node = split_str(str);
    if (node == NULL) return false;

    char *buffer = NULL;
    long val = strtol(node, &buffer, 10);
    if (str == buffer) {
        fprintf(stderr,
                "[./generator] Error: NODE_ID was not an integer. \nUsage: ./generator [NODE_ID-NODE_ID] ... \n");
        return false;
    }
    arr[0] = val;

    node = split_str(NULL);
    val = strtol(node, &buffer, 10);
    if (str == buffer) {
        fprintf(stderr,
                "[./generator] Error: NODE_ID was not an integer. \nUsage: ./generator [NODE_ID-NODE_ID] ... \n");
        return false;
    }
    arr[1] = val;

    return true;
}

/**
 * @brief Checks if search_for is already contained in the array.
 *
 * @param array Array to be searched for.
 * @param array_size Size of the given array.
 * @param search_for Integer to be searched for.
 * @return True if item is already present, otherwise false.
 */
static bool is_present(const long *array, size_t array_size, long search_for) {
    for (int i = 0; i < array_size; ++i) {
        if (array[i] == search_for) return true;
    }
    return false;
}

/**
 * @brief Creates graph from command line arguments.
 * @details Argument counter has to be bigger then one so that there is at least one edge.
 * The graph is built implicitly by taking all nodes which that occur in the arguments.
 *
 * It's illegal to use numbers below 0 since the string would be malformed.
 *
 * The returned value has to be deleted with delete_graph()
 *
 * @param argc Argument counter.
 * @param argv Argument variable.
 * @return Graph with nodes and edges and memory allocated accordingly.
 */
static graph_t *create_graph_from_args(int argc, char **argv) {
    graph_t *graph = malloc(sizeof(graph_t));
    if (graph == NULL) {
        fprintf(stderr, "[./generator] Error allocating memory for the graph. \n");
        exit(EXIT_FAILURE);
    }

    if (argc <= 1) {
        free(graph);
        fprintf(stderr, "[./generator] Generator needs at least one edge. \n");
        exit(EXIT_FAILURE);
    }

    /** Create temporary arrays for the nodes */
    size_t edges = (argc - 1);
    size_t min_nodes = (argc - 1) * 2;
    long node_ids[min_nodes];
    long node_ids_distinct[min_nodes]; // We use the same size so we are on the safe side
    memset(node_ids_distinct, -1, sizeof(node_ids_distinct));

    /** Iterate through all edge definitions and create a structure */
    size_t pos = 0;
    size_t distinct_idx = 0;
    for (int i = 1; i < argc; ++i, pos += 2) {
        if (!parse_numbers(argv[i], &node_ids[pos])) {
            free(graph);
            fprintf(stderr, "[./generator] Error while parsing arguments. \n");
            exit(EXIT_FAILURE);
        }

        long node_id_1 = node_ids[pos];
        long node_id_2 = node_ids[pos + 1];

        /** create distinct array of node_ids **/
        if (!is_present(node_ids_distinct, min_nodes, node_id_1)) {
            node_ids_distinct[distinct_idx++] = node_id_1;
        }
        if (!is_present(node_ids_distinct, min_nodes, node_id_2)) {
            node_ids_distinct[distinct_idx++] = node_id_2;
        }
    }


    /** Set Distinct Nodes and edges to graph */
    graph->nodes = malloc(sizeof(node_t) * distinct_idx);
    graph->node_count = distinct_idx;
    for (int i = 0; i < distinct_idx; ++i) {
        graph->nodes[i].color = 0;
        graph->nodes[i].id = node_ids_distinct[i];
    }

    graph->edges = malloc(sizeof(edge_t) * edges);
    graph->edge_count = edges;
    pos = 0;
    for (int i = 0; i < edges; i++, pos += 2) {
        node_t *node1 = find_node_by_id(graph, node_ids[pos]);
        node_t *node2 = find_node_by_id(graph, node_ids[pos + 1]);

        assert(node1 != NULL && node2 != NULL);

        graph->edges[i].node1 = node1;
        graph->edges[i].node2 = node2;
    }

    return graph;
}


/**
 * @brief Main entry point for generator program.
 * @details Main function.
 *
 * @param argc
 * @param argv
 * @return exit code
 */
int main(int argc, char **argv) {
    /** Initalize seed for random number generator */
    struct timeval t1;
    gettimeofday(&t1, NULL);
    srand(t1.tv_usec * t1.tv_sec + getpid());

    /** Create graph from command line arguments */
    graph_t *graph = create_graph_from_args(argc, argv);

    /** Open circular buffer as client */
    circular_buffer_t *cbuff = open_cbuff(false);
    if (cbuff == NULL) {
        fprintf(stderr, "[./generator] Error opening Circular Buffer. \n");
        delete_graph(graph);
        exit(EXIT_FAILURE);
    }

    /** Generate random solutions until supervisor program halts */
    long buffer[graph->edge_count * 2];
    while (!cbuff->shm->halt) {
        color_randomly(graph);
        size_t edge_count = get_deletion_edges(graph, buffer);
        if (edge_count > MIN_BOUNDARY) continue;

        /** Terminate if there was some kind of big error with semaphores etc. */
        if (!add_solution(cbuff, buffer, edge_count * 2)) {
            break;
        }
    }

    /** Close circular buffer and delete graph */
    delete_graph(graph);
    if (!close_cbuff(cbuff, false)) {
        fprintf(stderr, "[./generator] ERROR: Couldnt close circular buffer. \n");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
