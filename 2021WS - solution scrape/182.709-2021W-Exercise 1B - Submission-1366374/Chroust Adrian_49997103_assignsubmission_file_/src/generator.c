/**
 * @file generator.c
 * @author Adrian Chroust <e12023146@student.tuwien.ac.at>
 * @date 8.11.2021
 * @brief Generator program module.
 * @details Entrance point for a program that generates a random feedback arc set
 *          and sends its solution to the supervisor module.
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "sharedcircularbuffer.h"

/**
 * @brief The program name detected by the main function.
 * @details Required for printing out the proper usage of the program through the usage function.
 * */
static char *prog_name;

/**
 * @brief This function prints the expected input parameters of the program to stderr.
 * @details The function is usually called when a call with unexpected inputs is detected.
 */
static void usage(void) {
    fprintf(stderr, "[%s] Usage: %s EDGE1 EDGE2 ... where each edge is of the form v1-v2\n", prog_name, prog_name);
    fprintf(stderr, "[%s] Example: %s 1-2 3-4 5-6 7-8 9-10\n", prog_name, prog_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Atomic signal integer indicating if a signal was detected.
 */
static volatile sig_atomic_t quit = 0;

/**
 * @brief Signal handler updating the signal integer quit.
 */
static void handle_signal(int signal) {
    quit = 1;
}

/**
 * @brief The number of distinct vertices the user passed to the program.
 */
static unsigned int number_of_vertices = 0;

/**
 * @brief The number of edges the user passed to the program.
 */
static unsigned int number_of_edges = 0;

/**
 * @brief Dynamically allocated memory array for the indexes of the vertices passed to the program.
 */
static unsigned int *vertices;

/**
 * @brief Dynamically allocated memory array for the indexes of the edges passed to the program.
 * @details Each edge takes up two memory spaces and all edges are stored subsequently,
 *          meaning the array looks like {e1_v1, e1_v2, e2_v1, e2_v2, ...}.
 */
static unsigned int *edges;

/**
 * @brief Check if a string is a valid unsigned integer to the base 10.
 * @param str The string to check.
 * @return A boolean value indicating whether the input string is a valid uint to the base 10.
 */
static bool is_valid_uint10(char *str) {
    if (str == NULL) return false;

    unsigned long len = strlen(str);
    for (unsigned long i = 0; i < len; i++) {
        char c = str[i];
        // Check if the char is a value between 0 and 9
        if (c < '0' || c > '9') return false;
    }
    return true;
}

/**
 * @brief Adds a vertice to the vertices array.
 * @details Before adding the vertice, the vertices array is read to check if such vertice already exists.
 * @param v The vertice to add.
 */
static void add_vertice(unsigned int v) {
    for (unsigned int i = 0; i < number_of_vertices; i++) {
        if (v == vertices[i]) return;
    }

    vertices[number_of_vertices] = v;
    number_of_vertices++;
}

/**
 * @brief Adds an edge to the edges array.
 * @param v1 The head of the edge.
 * @param v2 The tail of the edge.
 */
static void add_edge(unsigned int v1, unsigned int v2) {
    unsigned int i = 2 * number_of_edges;
    edges[i] = v1;
    edges[i + 1] = v2;
    number_of_edges++;
}

/**
 * @brief Parses an edge string and adds it and its vertices to the vertices and edges arrays.
 * @param edge The edge string of the type "v1-v2".
 * @return -1 if the string does not comply with what is expected.
 */
static int parse_edge(char *edge) {
    char *v_str1 = strtok(edge, "-");
    if (!is_valid_uint10(v_str1)) return -1;

    char *v_str2 = strtok(NULL, "\0");
    if (!is_valid_uint10(v_str2)) return -1;

    char *end;
    unsigned long long v1 = strtol(v_str1, &end, 10);
    unsigned long long v2 = strtol(v_str2, &end, 10);

    // If the values are greater than UINT_MAX, they cannot be converted to unsigned int.
    // If the values are equal to UINT_MAX, they are equal the END_OF_WRITE constant needed
    // to indicate a write end in the shared circular buffer.
    if (v1 >= UINT_MAX || v2 >= UINT_MAX) return -1;

    add_vertice(v1);
    add_vertice(v2);
    add_edge(v1, v2);
    return 0;
}

/**
 * @brief Free all memory obtained to store the graph.
 */
static void free_graph(void) {
    free(vertices);
    free(edges);
}

/**
 * @brief Allocate the memory needed to store the graph and parse all edges.
 * @param edge_count The number of edges in the edge list.
 * @param edge_list A list of edges of the type "v1-v2".
 * @return -2 if an edge could not be parsed, -1 if dynamic memory could not be allocated and 0 if the parsing was successful.
 */
static int parse_graph(int edge_count, char *edge_list[]) {
    unsigned int size_unit = sizeof(unsigned int);
    unsigned int edges_size = 2 * edge_count * size_unit;

    // Allocate the memory required for the edges.
    edges = malloc(edges_size);
    if (edges == NULL) return -1;

    // Allocate the maximum memory required for the vertices.
    // The size same as edge_size as the maximum amount of distinct vertices
    // can not be greater than twice the edge count since every edge could at max hold to unique edges.
    vertices = malloc(edges_size);
    if (vertices == NULL) {
        free(edges);
        return -1;
    }

    // Parse all edges.
    for (int i = 0; i < edge_count; i++) {
        if (parse_edge(edge_list[i]) != 0) {
            free_graph();
            return -2;
        }
    }

    // Lower the size of memory for the vertices array to the actually required size.
    unsigned int *new_vertices = realloc(vertices, number_of_vertices * size_unit);
    if (new_vertices == NULL) {
        free_graph();
        return -1;
    }

    vertices = new_vertices;
    return 0;
}

/**
 * @brief Randomly shuffles the vertices in place.
 * @details Algorithm by Richard Durstenfeld, based on the Fisher-Yates shuffle.
 *          https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm
 */
static void shuffle_vertices() {
    for (unsigned int i = number_of_vertices - 1; i >= 1; i--) {
        // obtain a value between 0 and i
        unsigned int j = (i * rand()) / RAND_MAX;

        // swap values at indexes i and j
        unsigned int temp = vertices[i];
        vertices[i] = vertices[j];
        vertices[j] = temp;
    }
}

/**
 * @brief Writes a value to the shared circular buffer.
 * @details After a write the write_pos is incremented and set to 0 again if the end of the buffer is reached.
 *          If the free_space_sem is not incremented from 0 for a longer time, the write fails so the process
 *          can check if it is being signalled to stop.
 * @param val Value to write to the buffer.
 * @return -1 if the semaphores could not be read or updated, 0 otherwise.
 */
static int write_to_buffer(unsigned int val) {
    struct timespec ts;

    while (true) {
        // Get the current time.
        if (clock_gettime(CLOCK_REALTIME, &ts) != 0) return -1;
        ts.tv_sec += 1;

        // Wait for 1 second before retrying to get write access.
        int s = sem_timedwait(free_space_sem, &ts);

        if (s != 0 && errno == ETIMEDOUT) return -1;

        // If the error was a timeout the quit value is checked to see if the process should be terminated.
        // If so, the quit variable is set to 1 and the function returns with errors.
        if ((*shared_buffer).quit == 1) {
            quit = 1;
            return -1;
        }

        if (s == 0) break;
    }

    unsigned int write_pos = (*shared_buffer).write_pos;

    // Write the value to the buffer.
    (*shared_buffer).data[write_pos] = val;
    (*shared_buffer).write_pos = (write_pos + 1) % BUFFER_DATA_LENGTH;

    if (sem_post(used_space_sem) != 0) return -1;
    return 0;
}

/**
 * @brief Tries to write the solution to the buffer data.
 * @details If the write of any value fails, the write position
 * @return -1 if the write was unsuccessful or the process was signalled to quit and 0 if the write finished successfully.
 */
static int write_solution() {
    // As each edge stores 2 vertices, the double number of edges is used for iterating over all edges.
    unsigned int double_number_of_edges = 2 * number_of_edges;

    if (sem_wait(write_access_sem) != 0) return -1;

    // If the process was signalled to quit, the global quit variable is set to 1
    // and the function returns without any errors.
    if ((*shared_buffer).quit == 1) {
        if (sem_post(write_access_sem) != 0) return -1;
        quit = 1;
        return -1;
    }

    // Iterate over all edges.
    for (unsigned int i = 0; i < double_number_of_edges; i += 2) {
        unsigned int v1 = edges[i];
        unsigned int v2 = edges[i + 1];

        // Iterate over all vertices.
        for (unsigned int j = 0; j < number_of_vertices; j++) {
            unsigned int v = vertices[j];
            // If v2 of an edge comes before v1 the edge is added to the solution, otherwise the edge is left out.
            if (v == v1) break;
            if (v == v2) {
                if (write_to_buffer(v1) != 0) {
                    sem_post(write_access_sem);
                    return -1;
                }
                if (write_to_buffer(v2) != 0) {
                    sem_post(write_access_sem);
                    return -1;
                }
                break;
            }
        }
    }

    // Add the end of write value to the buffer to signal that the solution has been fully written.
    if (write_to_buffer(END_OF_WRITE) != 0) {
        sem_post(write_access_sem);
        return quit == -1;
    }

    if (sem_post(write_access_sem) != 0) return -1;
    return 0;
}

/**
 * @brief Program entry point.
 * @details Holds the main logic for the program which consists of generating a random solution for the feedback arc set
 *          as well as listening to Interrupts and signals from the main program to stop the execution.
 * @param argc The argument count of argv.
 * @param argv The arguments that the user inputted into the program. For more information please refer to the usage function.
 * @return returns EXIT_SUCCESS on successful execution and EXIT_FAILURE if unexpected or fatal errors occurred.
 */
int main(int argc, char *argv[]) {
    prog_name = argv[0];
    // The program expects at least one edge.
    if (argc == 1) usage();

    int f = parse_graph(argc - 1, &(argv[1]));

    // Refer to usage if an edge could not be parsed.
    if (f == -2) usage();
    // Return for other (memory) errors.
    if (f != 0) {
        fprintf(stderr, "[%s] Error: Could not allocate enough memory.\n", prog_name);
        return EXIT_FAILURE;
    }

    // Open the existing shared memory and all required semaphores.
    if (initialize_shared_circular_buffer(false) != 0) {
        free_graph();
        fprintf(stderr, "[%s] Error: Could not open shared circular buffer.\n", prog_name);
        return EXIT_FAILURE;
    }

    // Set the seed of the rand function to the current process id to make it unique.
    srand(process_id);

    // Listen to interrupts and termination signals.
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // As long as the process is not told to quit,
    // it will generate solutions and write them to the shared buffer.
    while (quit == 0) {
        // Create a random solution by shuffling the vertices of the graph.
        shuffle_vertices();

        // Try to write the solution function to the shared buffer.
        // If an error apart from a timeout or the process being told to quit occurs,
        // the function returns an error which has to be handled.
        if (write_solution() != 0) {
            if (quit == 1) break;

            fprintf(stderr, "[%s] Error: Could not write to shared circular buffer.\n", prog_name);
            free_graph();
            terminate_shared_circular_buffer(false);
            return EXIT_FAILURE;
        }
    }

    // Free up allocated and shared memory as well as semaphores.
    free_graph();
    if (terminate_shared_circular_buffer(false) != 0) {
        fprintf(stderr, "[%s] Error: Could not remove close circular buffer.\n", prog_name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
