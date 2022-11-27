/**
 * @file supervisor.c
 * @author Andreas Himmler 11901924
 * @date 12.11.2021
 *
 * @brief Supervises generators, that solutions with minimal edges removed,
 *        so that a graph is 3 colorable.
 */
#include "3coloring.h"

static char *program; /**< The name of the program.  */
volatile static sig_atomic_t quit = 0; /**< If the program should quit.  */

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
 * @brief Sets up the buffer and the semaphores to be available for all
 *        generators.
 * @details uses program, BUFFER_NAME, FREE_SPACE, USED_SPACE, MUTEX_NAME, MAX_EDGES
 * @return the process stuct containing all necessary shared memorys and semaphores.
 */
static process setup(void)
{
    process process;
    // Setup signal handlers:
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	if(check_sigaction(sigaction(SIGINT, &sa, NULL), program) == -1) exit(EXIT_FAILURE);
	if(check_sigaction(sigaction(SIGTERM, &sa, NULL), program) == -1) exit(EXIT_FAILURE);

	// Open shared memory:
	process.bufferfd = shm_open(BUFFER_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (check_buffer(process.bufferfd, program, BUFFER_NAME) == -1) exit(EXIT_FAILURE);
	if (ftruncate(process.bufferfd, sizeof(circular_buffer)) < 0) exit(EXIT_FAILURE);

	process.buffer = mmap(NULL, sizeof(*process.buffer), PROT_READ | PROT_WRITE, MAP_SHARED, process.bufferfd, 0);
	if(process.buffer == MAP_FAILED)
	{
		print_map_error(program, BUFFER_NAME);
		exit(EXIT_FAILURE);
	}

	// Open semaphores:
	process.free_space = sem_open(FREE_SPACE, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
	process.used_space = sem_open(USED_SPACE, O_CREAT | O_EXCL, 0600, 0);
	process.buff_mutex = sem_open(MUTEX_NAME, O_CREAT | O_EXCL, 0600, 1);

	if(check_sem_open(process.free_space, program, FREE_SPACE) == -1) exit(EXIT_FAILURE);
	if(check_sem_open(process.used_space, program, USED_SPACE) == -1) exit(EXIT_FAILURE);
	if(check_sem_open(process.buff_mutex, program, MUTEX_NAME) == -1) exit(EXIT_FAILURE);

    // Set starting values for the buffer.
	process.buffer->read_pos = 0;
	process.buffer->write_pos = 0;
	process.buffer->alive = 0;
	process.buffer->best_sol = MAX_EDGES + 1;

    return process;
}

/**
 * @brief Cleans up all open resources used in the program.
 * @details uses program, BUFFER_NAME, FREE_SPACE, USED_SPACE, MUTEX_NAME
 * @param process the process containing all semaphores and shared memorys.
 */
static void cleanup(process process)
{
    // Tell generators to terminate:
	process.buffer->alive = 1;
	if(check_sem_post(sem_post(process.free_space), program, FREE_SPACE) == -1) exit(EXIT_FAILURE);

	// Close shared memory:
	if (munmap(process.buffer, sizeof(*process.buffer)) == -1)
	{
		print_map_error(program, BUFFER_NAME);
		exit(EXIT_FAILURE);
	}
	if (check_close(close(process.bufferfd), program, BUFFER_NAME) == -1) exit(EXIT_FAILURE);
	if (check_buffer(shm_unlink(BUFFER_NAME), program, BUFFER_NAME) == -1) exit(EXIT_FAILURE);

	// Close semaphores:
	if (check_sem_close(sem_close(process.free_space), program, FREE_SPACE) == -1) exit(EXIT_FAILURE);
	if (check_sem_close(sem_close(process.used_space), program, USED_SPACE) == -1) exit(EXIT_FAILURE);
	if (check_sem_close(sem_close(process.buff_mutex), program, MUTEX_NAME) == -1) exit(EXIT_FAILURE);

	// Unlink semaphores:
	if(check_sem_unlink(sem_unlink(FREE_SPACE), program, FREE_SPACE) == -1) exit(EXIT_FAILURE);
	if(check_sem_unlink(sem_unlink(USED_SPACE), program, USED_SPACE) == -1) exit(EXIT_FAILURE);
	if(check_sem_unlink(sem_unlink(MUTEX_NAME), program, MUTEX_NAME) == -1) exit(EXIT_FAILURE);
}

/**
 * @brief Reads the circular buffer and outputs the first not already read
 *        solution to the graph provided to the generators.
 * @details uses BUFFER_SIZE
 * @param process the process containing all semaphores and shared memorys.
 * @return Edges to be removed from the graph to be 3 colorable.
 */
static solution cb_read(process process)
{
	solution ret = process.buffer->solutions[process.buffer->read_pos];
	process.buffer->read_pos += 1;
	if(process.buffer->read_pos >= BUFFER_SIZE) process.buffer->read_pos = 0;
	return ret;
}

/**
 * @brief Prints the usage of the program.
 * @details uses program
 */
static void usage(void)
{
	fprintf(stderr, "[%s] Usage: %s\n", program, program);
}

/**
 * @brief The main function. Processes all arguments and includes the main loop,
 *        that manages the buffer and reads new solutions.
 * @details uses program, quit, USED_SPACE, FREE_SPACE
 * @param argc Number of arguments.
 * @param argv arguments.
 * @return EXIT_SUCCESS if no error occurs.
 * @return EXIT_FAILURE in case of an error.
 */
int main(int argc, char *argv[])
{
	program = argv[0];

    // Argument parsing:
	if(argc > 1)
	{
		usage();
		exit(EXIT_FAILURE);
	}

    // Setup all resources:
	process process = setup();

    // Main loop:
	while(quit == 0)
	{
        // Wait if there are no new solutions:
		if(sem_wait(process.used_space) == -1)
		{
			if(errno == EINTR) continue;
			fprintf(stderr, "[%s] Semaphore %s is not valid.\n", program, USED_SPACE);
			exit(EXIT_FAILURE);
		}

        // Read new solution:
		solution sol = cb_read(process);

        // "Free" read memory:
		if(check_sem_post(sem_post(process.free_space), program, FREE_SPACE) == -1) exit(EXIT_FAILURE);

        // If number of edges are 0 => 3 colorable
		if (sol.count == 0 && !quit)
        {
			printf("[%s] The graph is 3-colorable!\n", program);
			break;
		}

        // If number of edges are less than previous => New best solution:
		if (sol.count < process.buffer->best_sol && !quit)
        {
			printf("[%s] Solution with %d edges:", program, sol.count);

			for (int i = 0; i < sol.count; i++)
            {
				printf(" %d-%d", sol.edges[i].v1, sol.edges[i].v2);
			}

			printf("\n");
			process.buffer->best_sol = sol.count;
		}
	}

    // Cleanup of resources:
	cleanup(process);
	exit(EXIT_SUCCESS);
}
