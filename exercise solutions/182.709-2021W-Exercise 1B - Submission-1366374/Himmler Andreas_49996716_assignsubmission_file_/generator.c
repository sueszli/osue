/**
 * @file generator.c
 * @author Andreas Himmler 11901924
 * @date 12.11.2021
 *
 * @brief Generates graphs with edges removed from the input so that the graph
 *        is 3-colorable.
 */
#include <regex.h>
#include <time.h>

#include "3coloring.h"

static char *program; /**< The name of the program.  */
volatile static sig_atomic_t quit = 0; /**< If the program should quit.  */


typedef struct graph {
    edge *edges;
    int last_vertex;
    int edge_count;
} graph;

/**
 * @brief Signal handler, that that tells the programm to terminate, if a signal
 *        has been read.
 * @details uses quit
 * @param signal Signal code.
 */
static void handle_signal(int signal)
{
	quit = 1;
}

/**
 * @brief Sets the edge in edges at position pos to the edge contained in arg.
 *        Exits with EXIT_FAILURE if the edge has no valid format.
 * @details uses program
 * @param arg Edge as an string
 * @param pos Position the edge has to be written to
 * @param regex Regex for the edge to be checked against
 * @param graph the graph for the edges to be set in
 */
static void set_edge(char *arg, int pos, regex_t regex, graph *graph)
{
    // Check format of the edge:
	if (regexec(&regex, arg, 0, NULL, 0) != 0)
    {
		fprintf(stderr, "[%s] Edge '%s' is not in the correct format ([0-9]+-[0-9]+)!\n", program, arg);
		exit(EXIT_FAILURE);
	}

    // Parse indices:
	char *v;
	graph->edges[pos].v1 = strtol(arg, &v, 10);
	graph->edges[pos].v2 = -1 * strtol(v, NULL, 10);

    // Check if there is a new maximum vertex:
	if (graph->edges[pos].v1 > graph->last_vertex) graph->last_vertex = graph->edges[pos].v1;
	if (graph->edges[pos].v2 > graph->last_vertex) graph->last_vertex = graph->edges[pos].v2;
}

/**
 * @brief Colors each vertex randomly and removes all edges that are between
 *        2 identically colores vertices. The set of removed edges is the
 *        solution for an 3 colorable graph.
 * @details uses MAX_EDGES
 * @param graph the original graph
 * @return Set of edges to be removed to be 3 colorable.
 */
static solution generate_solution(graph *graph)
{
	solution sol;
	sol.count = 0;

	int vertices[graph->last_vertex + 1];

    // Color vertices randomly:
	for (int i = 0; i <= graph->last_vertex; i++)
    {
		vertices[i] = rand() % 3;
	}

    // Check all edges:
	for (int i = 0; i < graph->edge_count; i++)
    {
		if (vertices[graph->edges[i].v1] == vertices[graph->edges[i].v2])
        {
			sol.edges[sol.count] = graph->edges[i];
			sol.count += 1;
		}

		if(sol.count > MAX_EDGES) break;
	}

	return sol;
}

/**
 * @brief Sets up all needed resources to be available to the program.
 * @details uses program, BUFFER_NAME, FREE_SPACE, USED_SPACE, MUTEX_SPACE
 * @return a process struct including the semaphores and shared memory
 */
static process setup(void)
{
    process process;
	// Signal handling:
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	if(check_sigaction(sigaction(SIGINT, &sa, NULL), program) == -1) exit(EXIT_FAILURE);
	if(check_sigaction(sigaction(SIGTERM, &sa, NULL), program) == -1) exit(EXIT_FAILURE);

	// Open shared memory:
	process.bufferfd = shm_open(BUFFER_NAME, O_RDWR, 0);
	if (check_buffer(process.bufferfd, program, BUFFER_NAME) == -1) exit(EXIT_FAILURE);

	process.buffer = mmap(NULL, sizeof(*process.buffer), PROT_READ | PROT_WRITE, MAP_SHARED, process.bufferfd, 0);
	if(process.buffer == MAP_FAILED)
	{
		print_map_error(program, BUFFER_NAME);
		exit(EXIT_FAILURE);
	}

	// Open semaphores:
	process.free_space = sem_open(FREE_SPACE, 0);
	process.used_space = sem_open(USED_SPACE, 0);
	process.buff_mutex = sem_open(MUTEX_NAME, 0);

	if(check_sem_open(process.free_space, program, FREE_SPACE) == -1) exit(EXIT_FAILURE);
	if(check_sem_open(process.used_space, program, USED_SPACE) == -1) exit(EXIT_FAILURE);
	if(check_sem_open(process.buff_mutex, program, MUTEX_NAME) == -1) exit(EXIT_FAILURE);

    return process;
}

/**
 * @brief Cleans up all open resources used by the program.
 * @details uses program, BUFFER_NAME, FREE_SPACE, USED_SPACE, MUTEX_NAME
 * @param the process including the semaphores and the shared memory.
 *
 */
static void cleanup(graph *graph, process process)
{
	free(graph->edges);
    free(graph);

	// Close shared memory
	if(munmap(process.buffer, sizeof(*process.buffer)) == -1)
	{
		print_map_error(program, BUFFER_NAME);
		exit(EXIT_FAILURE);
	}
	if(check_close(close(process.bufferfd), program, BUFFER_NAME) == -1) exit(EXIT_FAILURE);

	// Close semaphores:
	if (check_sem_close(sem_close(process.free_space), program, FREE_SPACE) == -1) exit(EXIT_FAILURE);
	if (check_sem_close(sem_close(process.used_space), program, USED_SPACE) == -1) exit(EXIT_FAILURE);
	if (check_sem_close(sem_close(process.buff_mutex), program, MUTEX_NAME) == -1) exit(EXIT_FAILURE);
}

/**
 * @brief Prints the usage of the program.
 * @details uses program
 */
static void usage(void)
{
	fprintf(stderr, "[%s] Usage %s edge [edge...]\n", program, program);
	fprintf(stderr, "     edge: [0-9]+-[0-9]+\n");
}

/**
 * @brief Compiles out of the arguments given to the program a more computer
 *        friendly version and sets it to edges.
 * @details uses program
 * @param argc Number of arguments given to the program.
 * @param argv arguments given to the program.
 * @return the compiled graph
 */
static graph *compile_edges(int argc, char *argv[])
{
    graph *graph = (struct graph*) malloc(sizeof(struct graph));
	graph->edges = (edge*) malloc((argc - 1) * sizeof(edge));
    graph->last_vertex = 0;

	if (graph->edges == NULL)
	{
		fprintf(stderr, "[%s] Memory to store the edges could not be allocated.\n", program);
		exit(EXIT_FAILURE);
	}

    regex_t regex;

	if (regcomp(&regex, "^[0-9]+-[0-9]+$", REG_EXTENDED) != 0)
	{
		fprintf(stderr, "[%s] Could not compile the regex '^[0-9]+-[0-9]+$' for edge recognition.\n", program);
		exit(EXIT_FAILURE);
	}

	for (int i = 1; i < argc; i++) set_edge(argv[i], i - 1, regex, graph);

	regfree(&regex);

    return graph;
}

/**
 * @brief Writes a solution to the buffer the moment it gets available.
 * @details uses program, FREE_SPACE, MUTEX_NAME, USED_SPACE, BUFFER_SIZE
 * @param sol The solution to be written to the buffer
 * @param process the process struct including the semaphores and shared memory
 * @return 0 if no error occured
 * @return -1 if the waiting for a semaphore was disrupted by a signal.
 */
static int cb_write(solution sol, process process)
{
    // Wait to be able to write:
	if(sem_wait(process.free_space) == -1)
	{
		if (errno == EINTR) return -1;
		fprintf(stderr, "[%s] Semaphore %s is not valid.\n", program, FREE_SPACE);
		exit(EXIT_FAILURE);
	}
	if(sem_wait(process.buff_mutex) == -1)
	{
		if (errno == EINTR) return -1;
		fprintf(stderr, "[%s] Semaphore %s is not valid.\n", program, MUTEX_NAME);
		exit(EXIT_FAILURE);
	}

    // Reserve a available writing position:
	int write_pos = process.buffer->write_pos;
	process.buffer->write_pos += 1;
	if(process.buffer->write_pos >= BUFFER_SIZE) process.buffer->write_pos = 0;

	if(check_sem_post(sem_post(process.buff_mutex), program, MUTEX_NAME) == -1) exit(EXIT_FAILURE);

    // Write to the buffer at given position:
	process.buffer->solutions[write_pos] = sol;

	if(check_sem_post(sem_post(process.used_space), program, USED_SPACE) == -1) exit(EXIT_FAILURE);

	return 0;
}

/**
 * @brief The main function. Manages the arguments and the flow of the program.
 *        Also contains the main loop, where solutions are generated and if
 *        better than any previous one be written to the buffer.
 * @details uses program, quit
 * @param argc Number of the arguments
 * @param argv arguments
 */
int main(int argc, char *argv[])
{
    // Argument handling:
	program = argv[0];
	if(argc <= 1)
	{
		usage();
		exit(EXIT_FAILURE);
	}

    // Setups:
	process process = setup();
	srand(time(0) ^ getpid());

	graph *graph = compile_edges(argc, argv);
    graph->edge_count = argc - 1;

    // Main loop:
	while(process.buffer->alive == 0 && quit == 0)
	{
		solution sol = generate_solution(graph);

		if (sol.count < process.buffer->best_sol)
        {
			if(cb_write(sol, process) == -1) continue;
		}
	}

    // Cleanup:
	cleanup(graph, process);
	exit(EXIT_SUCCESS);
}
