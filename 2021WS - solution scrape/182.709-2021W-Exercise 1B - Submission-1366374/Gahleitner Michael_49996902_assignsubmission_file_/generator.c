#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include "globals.h"
#include "shm_data.h"
#include "generator.h"

char *program_name;

/**
 * @brief Program entry
 * @details Starts genartor with edges passed as arguments and writes solutions to shared memory.
 * @param argc argument count
 * @param argv argument vector
 */
int main(int argc, char *argv[])
{
    program_name = argv[0];
    graph_data_t graph;
    graph.vertex_count = 0;
    graph.vertices = NULL;

    if (argc > 1)
    {
        graph.edge_count = argc - 1;
        graph.edges = parse_edges(graph.edge_count, argv + 1);
        extract_vertices(&graph);

        int best_solution = -1;
        init_sem_shm();
        srand(time(NULL));

        graph_data_t working_graph;
        working_graph.edge_count = graph.edge_count;
        working_graph.vertex_count = graph.vertex_count;

        while (1)
        {
            if (shared_memory->stop == 1)
            {
                handle_signal(SIGINT);
            }

            working_graph.edges = (edge_t *)malloc(sizeof(edge_t) * graph.edge_count);
            check_malloc(working_graph.edges);

            working_graph.vertices = (vertex_data_t *)malloc(sizeof(vertex_data_t) * graph.vertex_count);
            check_malloc(working_graph.vertices);

            memcpy(working_graph.edges, graph.edges, sizeof(edge_t) * graph.edge_count);
            memcpy(working_graph.vertices, graph.vertices, sizeof(vertex_data_t) * graph.vertex_count);

            color_vertices(&working_graph);

            edge_t *removed_edges = (edge_t *)malloc(sizeof(edge_t) * 0);
            check_malloc(removed_edges);

            int removed_edges_count = remove_color_pairs(&working_graph, removed_edges);

            if (((removed_edges_count < best_solution) || best_solution == -1) && removed_edges_count <= 8)
            {
                sem_wait(sem_write_index);
                if (errno == EINTR)
                {
                    fprintf(stderr, "%s EINTR - shutton down: %s\n", program_name, strerror(errno));
                    exit_error();
                }
                sem_wait(sem_free_space);
                if (errno == EINTR)
                {
                    fprintf(stderr, "%s EINTR - shutton down: %s\n", program_name, strerror(errno));
                    exit_error();
                }
                if (shared_memory->stop == 1)
                {
                    handle_signal(SIGINT);
                }

                shared_memory->solutions[*shared_write_index][0].from = -1;
                shared_memory->solutions[*shared_write_index][0].to = removed_edges_count;

                for (int i = 1; i <= removed_edges_count; i++)
                {
                    shared_memory->solutions[*shared_write_index][i].from = removed_edges[i - 1].from;
                    shared_memory->solutions[*shared_write_index][i].to = removed_edges[i - 1].to;
                }

                if (*shared_write_index == MAX_BUFFER_INDEX)
                {
                    *shared_write_index = 0;
                }
                else
                {
                    *shared_write_index += 1;
                }
                best_solution = removed_edges_count;
                sem_post(sem_used_space);
                sem_post(sem_write_index);
            }

            free(working_graph.edges);
            free(working_graph.vertices);
        }
    }
    else
    {
        usage();
    }
}

/**
 * @details Parses all the input arguments to generate edges
 * @param count_edges amount of edges to parse
 * @param input_edges string array of edges to parse
 * @return Returns list of parsed edges.
 */
edge_t *parse_edges(int count_edges, char *input_edges[])
{
    edge_t *edges = (edge_t *)malloc(sizeof(edge_t) * count_edges);
    check_malloc(edges);

    for (int i = 0; i < count_edges; i++)
    {
        edges[i] = parse_edge(input_edges[i]);
    }

    return edges;
}

/**
 * @details Parses the input string of a edge and returns it as a edge
 * @param input_edge string to parse edge from
 * @return parsed edge
 */
edge_t parse_edge(char *input_edge)
{
    edge_t edge;
    char *remainder;

    edge.from = strtol(input_edge, &remainder, 10);
    remainder++;
    edge.to = strtol(remainder, NULL, 10);

    return edge;
}

/**
 * @details Extracts vertices of a graph and adds it to the vertex list of the graph data.
 * @param graph data
 */
void extract_vertices(graph_data_t *graph)
{
    graph->vertices = NULL;
    for (int i = 0; i < graph->edge_count; i++)
    {
        check_and_add_vertex(graph, graph->edges[i].from);
        check_and_add_vertex(graph, graph->edges[i].to);
    }
}

/**
 * @details checks whether a vertex is listed in a graph and adds the vertex in case it
 * is not in the graph
 * @param graph data
 * @param vertex to add
 */
void check_and_add_vertex(graph_data_t *graph, vertex_t vertex)
{
    if (!contains_vertex(graph, vertex))
    {
        graph->vertex_count++;
        graph->vertices = (vertex_data_t *)realloc(graph->vertices, sizeof(vertex_data_t) * graph->vertex_count);
        check_realloc(graph->vertices);
        graph->vertices[graph->vertex_count - 1].vertex = vertex;
    }
}

/**
 * @details checks whether a graph contains a vertex or not
 * @param graph data
 * @param vertex to find
 * @return Returns one in case vertex is in graph. Returns 0 otherwise.
 */
int contains_vertex(graph_data_t *graph, vertex_t vertex)
{
    for (int i = 0; i < graph->vertex_count; i++)
    {
        if (graph->vertices[i].vertex == vertex)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @details Randomly colors vertices in a graph
 * @param graph data
 */
void color_vertices(graph_data_t *graph)
{
    for (int i = 0; i < graph->vertex_count; i++)
    {
        graph->vertices[i].color = rand() % 2;
    }
}

/**
 * @details Removes the edges in a graph where both vertices have the same color
 * @param graph data
 * @param removed_edges pointer where removed edges are stored
 * @return amount of edges which were removed
 */
int remove_color_pairs(graph_data_t *graph, edge_t *removed_edges)
{
    int edge_count = 0;
    for (int i = 0; i < graph->edge_count; i++)
    {
        if (get_vertex_color(graph, graph->edges[i].from) == get_vertex_color(graph, graph->edges[i].to) && !contains_edge(removed_edges, edge_count, graph->edges[i]))
        {
            edge_count++;
            removed_edges = (edge_t *)realloc(removed_edges, sizeof(edge_t) * edge_count);
            check_realloc(removed_edges);

            memcpy(removed_edges + (edge_count - 1), graph->edges + i, sizeof(edge_t));

            *(graph->edges + i) = *(graph->edges + i + 1);
            graph->edge_count--;
            --i;
        }
    }

    return edge_count;
}

/**
 * @details Check whether a edge is contained in a list of edges.
 * @param edges list of edges
 * @param edges_count amount of edges
 * @param edge edge to search
 * @return 1 if edge was found. Returns 0 otherwise.
 */
int contains_edge(edge_t *edges, int edges_count, edge_t edge)
{
    for (int i = 0; i < edges_count; i++)
    {
        if (edges[i].from == edge.from && edges[i].to == edge.to)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @details Returns a vector's color.
 * @param graph data
 * @param vector to find color of
 * @return the color of a vertex. Returns -1 in case vertex was not found.
 */
int get_vertex_color(graph_data_t *graph, vertex_t vertex)
{
    for (int i = 0; i < graph->vertex_count; i++)
    {
        if (graph->vertices[i].vertex == vertex)
        {
            return graph->vertices[i].color;
        }
    }
    return -1;
}

/**
 * @brief Prints usage
 * @details Prints usage which displays the usage of options and arguments and exits with EXIT_FAILURE status.
 */
void usage(void)
{
    fprintf(stderr, "Usage: %s\n EDGE1 EDGE2...", program_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Handles malloc error.
 * @details Prints the error caused by malloc to stderr and error exits.
 */
void check_malloc(void *pointer)
{
    if (pointer == NULL)
    {
        fprintf(stderr, "%s ERROR: malloc failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
}

/**
 * @brief Handles realloc error.
 * @details Prints the error caused by realloc to stderr and error exits.
 */
void check_realloc(void *pointer)
{
    if (pointer == NULL)
    {
        fprintf(stderr, "%s ERROR: realloc failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Inits semaphore and shared memory
 * @details Opens shared memory and semaphores while also handling possible
 * errors during initiation.
 */
void init_sem_shm(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);

    sem_free_space = sem_open(FREE_SPACE, 0);
    sem_used_space = sem_open(USED_SPACE, 0);
    sem_write_index = sem_open(WRITE_INDEX, 0);

    shm_fd = shm_open(SHARED_MEM, O_RDWR, 0600);
    if (shm_fd == -1)
    {
        fprintf(stderr, "%s ERROR: shm_open failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
    if (ftruncate(shm_fd, sizeof *shared_memory) == -1)
    {
        fprintf(stderr, "%s ERROR: ftruncate failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
    shared_memory = mmap(NULL, sizeof *shared_memory, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED)
    {
        fprintf(stderr, "%s ERROR: mmap failed: %s\n", program_name, strerror(errno));
        exit_error();
    }

    shm_write_idx_fd = shm_open(WRITE_INDEX, O_RDWR, 0600);
    if (shm_write_idx_fd == -1)
    {
        fprintf(stderr, "%s ERROR: shm_open failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
    if (ftruncate(shm_write_idx_fd, sizeof *shared_write_index) == -1)
    {
        fprintf(stderr, "%s ERROR: ftruncate failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
    shared_write_index = mmap(NULL, sizeof *shared_write_index, PROT_READ | PROT_WRITE, MAP_SHARED, shm_write_idx_fd, 0);
    if (shared_write_index == MAP_FAILED)
    {
        fprintf(stderr, "%s ERROR: mmap failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
}

/**
 * @brief Handles signal
 * @details Frees memory and exits.
 */
void handle_signal(int signal)
{
    free_memory();
    exit(EXIT_SUCCESS);
}

/**
 * @brief Error exit
 * @details Frees memory and exits with EXIT_FAILURE
 */
void exit_error(void)
{
    free_memory();
    exit(EXIT_FAILURE);
}

/**
 * @brief Frees the memory
 * @details Unmaps and closes shared memory. Closes semaphores
 */
void free_memory(void)
{
    if (munmap(shared_memory, sizeof *shared_memory) == -1)
    {
        fprintf(stderr, "%s ERROR: munmap failed: %s\n", program_name, strerror(errno));
    }
    if (munmap(shared_write_index, sizeof *shared_write_index) == -1)
    {
        fprintf(stderr, "%s ERROR: munmap failed: %s\n", program_name, strerror(errno));
    }

    handle_close(shm_fd);
    handle_close(shm_write_idx_fd);

    handle_sem_close(sem_free_space);
    handle_sem_close(sem_used_space);
    handle_sem_close(sem_write_index);
}

/**
 * @brief Close fd
 * @details Closes file descriptor and prints error, if occured during closing
 * @param fd file descriptor
 */
void handle_close(int fd)
{
    if (close(fd) == -1)
    {
        fprintf(stderr, "%s ERROR: close failed: %s\n", program_name, strerror(errno));
    }
}

/**
 * @brief Closes semaphore
 * @details Closes semaphore and prints error, if occured during unlinking
 * @param sem file descriptor
 */
void handle_sem_close(sem_t *sem)
{
    if (sem_close(sem) == -1)
    {
        fprintf(stderr, "%s ERROR: sem_close failed: %s\n", program_name, strerror(errno));
    }
}