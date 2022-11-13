/**
 * @author Peter Kabelik (01125096)
 * @file generator.c
 * @date 10.11.2021
 *
 * @brief A program to generate possible solutions for minimum feedback arc sets.
 * 
 * @details This program generates possible solutions for the for a minimum
 * feedback arc set for a given graph, wich will be provided by this programs.
 * The generator uses the shared memory and semaphores provided by the supervisor.
 * This program continuously creates feedback arc sets. If the arc set has
 * not too many edges, then it is considered as possible solution, which is
 * then written into the circular buffer (in the shared memory).
 * If the supervisor tells this program to terminate, it will do so after cleaning
 * up its part.
 **/

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <limits.h>
#include <regex.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

struct graph
{
	int* nodes;
	edge_t* edges;
	unsigned int node_count;
	unsigned int edge_count;
};

typedef struct graph graph_t;

/**
 * @brief Prints the usage description and exits.
 *
 * @details Prints the usage description including the program's name
 * to stdout and exits with EXIT_FAILURE.
 *
 * @param program_name The program's name.
 */
static void print_usage_and_exit(char* program_name)
{
	fprintf(stdout, "Usage: %s EDGE1...\nExample: %s 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0\n", program_name, program_name);
	
	exit(EXIT_FAILURE);
}

/**
 * @brief Checks if node is contained in the graph.
 *
 * @details Checks if a node is contained in the given graph.
 *
 * @param node The node to check.
 * @param graph The pointer to the graph to use for the check.
 *
 * @return Returns true, if the node is already part of the graph, false otherwise.
 */
static bool is_node_contained(int node, graph_t* graph)
{
	for (unsigned int i = 0; i < graph->node_count; i++)
	{
		if (node == graph->nodes[i])
		{
			return true;
		}
	}
	
	return false;
}

/**
 * @brief Adds node to the graph.
 *
 * @details Adds a node to the given graph, if it is not already
 * contained.
 *
 * @param node The node to add.
 * @param graph The pointer to the graph to which to add the node.
 */
static void add_node(int node, graph_t* graph)
{
	if (!is_node_contained(node, graph))
	{
		graph->nodes[graph->node_count] = node;
	
		graph->node_count++;
	}
}

/**
 * @brief Extract edges and nodes.
 *
 * @details Extracts the edges and nodes from the program argument vector
 * and add them to the given graph.
 *
 * @param argc The argument counter.
 * @param argv The argument vector containing the edges as positional arguments.
 * @param graph The pointer to the graph to use as extraction target.
 *
 * @return Returns true, if none of the given edges are malformed, false otherwise.
 */
static bool extract_edges_and_nodes(int argc, char* argv[], graph_t* graph)
{
	regex_t pattern_buffer;
	
	if (regcomp(&pattern_buffer, "^[0-9]+-[0-9]+$", REG_EXTENDED) != 0)
	{
		print_error(argv[0], "regcomp");
	
		exit(EXIT_FAILURE);
	}
	
	for (int i = 1; i < argc; i++)
	{
		if (regexec(&pattern_buffer, argv[i], 0, NULL, 0) != 0)
		{
			print_custom_error(argv[0], "at least one edge is malformed");
			
			regfree(&pattern_buffer);
			
			return false;
		}
		
		edge_t* edge = &graph->edges[i - 1];
		
		sscanf(argv[i], "%u-%u", &edge->start, &edge->end);
		
		add_node(edge->start, graph);
		add_node(edge->end, graph);
	}
	
	regfree(&pattern_buffer);
	
	return true;
}

/**
 * @brief Cleans graph.
 *
 * @details Frees the resources used by the graph.
 *
 * @param graph The pointer to the graph to use.
 *
 */
static void clean_graph(graph_t* graph)
{
	free(graph->edges);
	free(graph->nodes);
}

/**
 * @brief Initialises graph.
 *
 * @details Initialised the given graph.
 *
 * @param argc The argument counter.
 * @param argv The argument vector containing the edges as positional arguments.
 * @param graph The pointer to the graph to use as initialisation target.
 */
static void initialise_graph(int argc, char* argv[], graph_t* graph)
{
	graph->edge_count = argc - 1;
	graph->node_count = graph->edge_count * 2;
	
	graph->nodes = malloc(graph->node_count * sizeof(int));
	
	if (graph->nodes == NULL)
	{
		print_error(argv[0], "malloc");
		
		exit(EXIT_FAILURE);
	}
	
	graph->edges = malloc(graph->edge_count * sizeof(edge_t));
	
	if (graph->edges == NULL)
	{
		print_error(argv[0], "malloc");
		
		exit(EXIT_FAILURE);
	}
	
	for (int i = 0; i < graph->node_count; i++)
	{
		graph->nodes[i] = -1;
	}
	
	graph->node_count = 0;
	
	if (!extract_edges_and_nodes(argc, argv, graph))
	{
		clean_graph(graph);
		
		print_usage_and_exit(argv[0]);
	}
}

/**
 * @brief Shuffles nodes of graph.
 *
 * @details Shuffles the nodes of a given graph.
 *
 * @param graph The pointer to the graph to use as initialisation target.
 */
static void shuffle_nodes(graph_t* graph)
{
	for (unsigned int i = graph->node_count - 1; i > 0; i--)
	{
		int random_index = rand() % (i + 1);
		
		int i_node = graph->nodes[i];
		
		graph->nodes[i] = graph->nodes[random_index];
		graph->nodes[random_index] = i_node;
	}
}

/**
 * @brief Extract feedback arc set.
 *
 * @details Extracts an feedback arc set from the given graph.
 *
 * @param graph The pointer to the graph from which is extracted.
 * @param feedback_arc_set The pointer to the feedback_arc_set to use as extraction target.
 *
 * @return Returns true, if there are not too many edges in the set, false otherwise.
 */
static bool extract_feedback_arc_set(graph_t* graph, feedback_arc_set_t* feedback_arc_set)
{
	feedback_arc_set->edge_count = 0;

	for (unsigned int i = 0; i < graph->edge_count; i++)
	{
		edge_t* edge = &graph->edges[i];
		
		for (unsigned int j = 0; j < graph->node_count; j++)
		{
			int node = graph->nodes[j];
		
			if (edge->end == node)
			{
				if (feedback_arc_set->edge_count == MAX_FEEDBACK_ARC_SET_SIZE)
				{		
					return false; /* The solution has too many edges. */
				}
				
				edge_t* set_edge = &feedback_arc_set->edges[feedback_arc_set->edge_count];
				
				set_edge->start = edge->start;
				set_edge->end = edge->end;
			
				feedback_arc_set->edge_count++;
				
				break;
			}
		
			if (edge->start == node)
			{
				break;
			}
		}
	}
	
	return true;
}

/**
 * @brief The program's' entry point.
 *
 * @details After handling the program arguments, the shared memory and
 * the semaphores are initialised. If no errors occurred, solutions are
 * continuously written into the circular buffer (in the shared memory).
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 *
 * @return Returns EXIT_SUCCESS, if no errors occurred.
 */
int main(int argc, char* argv[])
{
	srand(time(NULL) + getpid());

	if (getopt(argc, argv, "") != -1)
	{
		print_usage_and_exit(argv[0]);
	}
	
	if (argc < 2)
	{
		print_custom_error(argv[0], "at least one edge has to be given");
		print_usage_and_exit(argv[0]);
	}
	
	shared_memory_t* shared_memory = initialise_shared_memory(false, argv[0]);
	
	sem_t* freed_space_semaphore = initialise_semaphore(false, FREED_SPACE_SEMAPHORE_NAME, 0, argv[0]);
	
	if (freed_space_semaphore == SEM_FAILED)
	{
		clean_shared_memory(false, shared_memory, argv[0]);
	}
	
	sem_t* used_space_semaphore = initialise_semaphore(false, USED_SPACE_SEMAPHORE_NAME, 0, argv[0]);
		
	if (used_space_semaphore == SEM_FAILED)
	{
		clean_semaphore(false, freed_space_semaphore, USED_SPACE_SEMAPHORE_NAME, argv[0]);
		
		clean_shared_memory(false, shared_memory, argv[0]);
	}
	
	sem_t* mutex_semaphore = initialise_semaphore(false, MUTEX_SEMAPHORE_NAME, 0, argv[0]);
		
	if (mutex_semaphore == SEM_FAILED)
	{
		clean_semaphore(false, used_space_semaphore, FREED_SPACE_SEMAPHORE_NAME, argv[0]);
		clean_semaphore(false, freed_space_semaphore, USED_SPACE_SEMAPHORE_NAME, argv[0]);
		
		clean_shared_memory(false, shared_memory, argv[0]);
	}
	
	shared_memory->generator_count++;
	
	graph_t graph;
	
	initialise_graph(argc, argv, &graph);

	while (!shared_memory->should_quit)
	{
		shuffle_nodes(&graph);
		
		feedback_arc_set_t feedback_arc_set;
		
		if (!extract_feedback_arc_set(&graph, &feedback_arc_set))
		{
			continue;
		}
		
		if (sem_wait(freed_space_semaphore) == -1)
		{
			print_error(argv[0], "sem_wait");
			
			break;
		}
		
		if (sem_wait(mutex_semaphore) == -1)
		{
			print_error(argv[0], "sem_wait");
			
			break;
		}
		
		shared_memory->circular_buffer[shared_memory->writing_position] = feedback_arc_set;
		
		shared_memory->writing_position++;
		shared_memory->writing_position %= CIRCULAR_BUFFER_SIZE;
		
		if (sem_post(mutex_semaphore) == -1)
		{
			print_error(argv[0], "sem_post");
			
			break;
		}
		
		if (sem_post(used_space_semaphore) == -1)
		{
			print_error(argv[0], "sem_post");
			
			break;
		}
	}
	
	clean_graph(&graph);
	
	clean_semaphore(false, mutex_semaphore, MUTEX_SEMAPHORE_NAME, argv[0]);
	clean_semaphore(false, used_space_semaphore, FREED_SPACE_SEMAPHORE_NAME, argv[0]);
	clean_semaphore(false, freed_space_semaphore, USED_SPACE_SEMAPHORE_NAME, argv[0]);
	
	shared_memory->generator_count--;
	
	clean_shared_memory(false, shared_memory, argv[0]);

	return EXIT_SUCCESS;
}

