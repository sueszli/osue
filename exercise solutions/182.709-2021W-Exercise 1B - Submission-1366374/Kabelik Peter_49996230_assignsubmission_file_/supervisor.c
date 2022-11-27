/**
 * @author Peter Kabelik (01125096)
 * @file supervisor.c
 * @date 11.11.2021
 *
 * @brief A program to supervise the search for minimum feedback arc sets.
 * 
 * @details This program supervises the search for a minimum feedback arc set for a
 * given graph, wich will be provided by generator programs. The supervisor has
 * to take care of creating and destroying shared memory and semaphores, which
 * both will also be used by the generator programs.
 * This program continuously checks the circular buffer for better solutions.
 * If one is found, then it is saved as new best solution and it will also be
 * displayed on the screen. If the given graph is already acyclic, then the
 * supervisor will eventually terminate, otherwise the user has to interrupt
 * when the current best solution seems satisfying enough. Either way, all the
 * generator processes will be told to also terminate.
 *
 * @details This program uses the global variable 'should_quit', which is
 * of the type 'sig_atomic_t'. It is used in the signal handler.
 **/

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

volatile sig_atomic_t should_quit = 0;

/**
 * @brief Handles a signal.
 *
 * @details Handles a signal by setting the global variable 'should_quit'.
 *
 * @param signal The type of signal beeing handled.
 */
static void handle_signal(int signal)
{
	should_quit = 1;
}

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
	fprintf(stdout, "Usage: %s\n", program_name);
	
	exit(EXIT_FAILURE);
}

/**
 * @brief Prints the solution.
 *
 * @details Prints the given solution.
 *
 * @param solution The pointer to the solution to print.
 * @param program_name The program's name.
 */
static void print_solution(feedback_arc_set_t* solution, char* program_name)
{
	if (solution->edge_count > 0)
	{
		fprintf(stdout, "[%s] Solution with %u edges:", program_name, solution->edge_count);
		
		for (unsigned int i = 0; i < solution->edge_count; i++)
		{
			edge_t* edge = &solution->edges[i];
		
			fprintf(stdout, " %u-%u", edge->start, edge->end);
		}
		
		fprintf(stdout, "\n");					
	}
	else
	{
		fprintf(stdout, "[%s] The graph is acyclic!\n", program_name);
	}
}

/**
 * @brief The program's' entry point.
 *
 * @details After handling the program arguments, the shared memory and
 * the semaphores are created and initialised. Then the signal handlers
 * are set up. If no errors occurred, the circular buffer (in the shared
 * memory) is continuously checked for a better solution than the current
 * one. 
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 *
 * @return Returns EXIT_SUCCESS, if no errors occurred.
 */
int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		print_custom_error(argv[0], "no arguments allowed");
		print_usage_and_exit(argv[0]);
	}

	shared_memory_t* shared_memory = initialise_shared_memory(true, argv[0]);
	
	sem_t* freed_space_semaphore = initialise_semaphore(true, FREED_SPACE_SEMAPHORE_NAME, CIRCULAR_BUFFER_SIZE, argv[0]);
	
	if (freed_space_semaphore == SEM_FAILED)
	{
		clean_shared_memory(true, shared_memory, argv[0]);
	}
	
	sem_t* used_space_semaphore = initialise_semaphore(true, USED_SPACE_SEMAPHORE_NAME, 0, argv[0]);
		
	if (used_space_semaphore == SEM_FAILED)
	{
		clean_semaphore(true, freed_space_semaphore, USED_SPACE_SEMAPHORE_NAME, argv[0]);
		
		clean_shared_memory(true, shared_memory, argv[0]);
	}
	
	sem_t* mutex_semaphore = initialise_semaphore(true, MUTEX_SEMAPHORE_NAME, 1, argv[0]);
		
	if (mutex_semaphore == SEM_FAILED)
	{
		clean_semaphore(true, used_space_semaphore, FREED_SPACE_SEMAPHORE_NAME, argv[0]);
		clean_semaphore(true, freed_space_semaphore, USED_SPACE_SEMAPHORE_NAME, argv[0]);
		
		clean_shared_memory(true, shared_memory, argv[0]);
	}
	
	struct sigaction signal_action = { .sa_handler = handle_signal };
	
	bool signal_action_error_occurred = false;
	
	if (sigaction(SIGINT, &signal_action, NULL) == -1)
	{
		print_error(argv[0], "sigaction");
	
		signal_action_error_occurred = true;
	}
	
	if (sigaction(SIGTERM, &signal_action, NULL) == -1)
	{
		print_error(argv[0], "sigaction");
		
		signal_action_error_occurred = true;
	}
	
	if (!signal_action_error_occurred)
	{
		feedback_arc_set_t best_solution = { .edge_count = MAX_FEEDBACK_ARC_SET_SIZE + 1 };
		
		unsigned int reading_position = 0;
		
		// shared_memory->writing_position = 0;
		// shared_memory->generator_count = 0;
		// shared_memory->should_quit = false;
	
		while (should_quit == 0)
		{
			if (sem_wait(used_space_semaphore) == -1)
			{
				if (errno != EINTR)
				{
					print_error(argv[0], "sem_wait");
				}
				
				break;
			}
			
			feedback_arc_set_t* solution = &shared_memory->circular_buffer[reading_position];
			
			if (solution->edge_count < best_solution.edge_count)
			{
				best_solution = *solution;
				
				print_solution(&best_solution, argv[0]);
			}
			
			if (sem_post(freed_space_semaphore) == -1)
			{
				print_error(argv[0], "sem_post");
				
				break;
			}
			
			reading_position++;
			reading_position %= CIRCULAR_BUFFER_SIZE;
			
			if (best_solution.edge_count == 0)
			{
				break;
			}
		}
		
		shared_memory->should_quit = true;
		
		while (shared_memory->generator_count > 0)
		{
			if (sem_post(freed_space_semaphore) == -1)
			{
				print_error(argv[0], "sem_post");
				
				break;
			}
		}
	}
	
	clean_semaphore(true, mutex_semaphore, MUTEX_SEMAPHORE_NAME, argv[0]);
	clean_semaphore(true, used_space_semaphore, FREED_SPACE_SEMAPHORE_NAME, argv[0]);
	clean_semaphore(true, freed_space_semaphore, USED_SPACE_SEMAPHORE_NAME, argv[0]);
	
	clean_shared_memory(true, shared_memory, argv[0]);
	
	return EXIT_SUCCESS;	
}
