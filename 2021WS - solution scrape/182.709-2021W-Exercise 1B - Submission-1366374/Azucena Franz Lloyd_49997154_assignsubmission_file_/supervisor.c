#include "supervisor.h"

/**
 * @file supervisor.c
 * @author Franz Lloyd Azucena <e1425044@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief supervisor
 *
 * @details sets up the shared memory and the semaphores and initializes the circular buffer required for the communication with the generators. It then waits for the generators to write solutions to the circular buffer.
 * */

/* indicates if this programm should terminate */
bool termination_bool = false;

void sighandler(int sig) {
	termination_bool = true;
}

void terminate_with_error_2(char* message) {
	print_error(name, message);
	exit(EXIT_FAILURE);
}

/**
 * @brief main funtion of supervisor
 * @details creates shared buffer, sets up the shared memory and the semaphores; on termination the cleanup is initialized here as well.
 * @param argc argument counter
 * @param argv array with arguments
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
 * */
int main(int argc, char* argv[]) {
	signal(SIGINT, sighandler);	// signal interrupt
	signal(SIGTERM, sighandler);	// termination request sent to program

	name = argv[0];

	if(argc != 1) {
		terminate_with_error_2("arguments not allowed");
	}

	init(false);

	result res;

	struct result best_result;

	while(!termination_bool) {
		bool success = read_buffer(&res);
		if(!success) {
			terminate_with_error_2("error occurred while reading from buffer");
		}

		if(count_edges(&res) == 0) {
			fprintf(stdout, "This graph is acyclic.\n");
			break;
		}

		if(count_edges(&res) < count_edges(&best_result)) {
			best_result = res;
			print_result(&best_result);
		}

		if(count_edges(&best_result)==0) {
			// best result found
			break; 
		}
	}
	
	sem_post(free_space_sem);

	terminate();
	exit(EXIT_SUCCESS);	
}
