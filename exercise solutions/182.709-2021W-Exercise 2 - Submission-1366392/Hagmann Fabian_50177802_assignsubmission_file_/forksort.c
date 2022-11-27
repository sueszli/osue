/**
 * @file forksort.c
 * @author Fabian Hagmann (12021352)
 * @brief Implementation of OSUE 2021W exercise 2 (forksort)
 * @details Reads input from stdin. If necessary, fork two child processes
 * that solve one half of the process and merge the two results afterwards.
 * Communication between parent and child processes is implemented via
 * pipes.
 * @date 2021-11-24
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

/**
 * struct to store file streams to a child compactly
 */
typedef struct {
	/*@{*/
	FILE *in; 	/**< input stream to the child */
	FILE *out;	/**< output stream from the child */
	pid_t id;	/**< process id of the child */
	/*@}*/
} child_t;

static char *store_line(char* line, int length);
static child_t fork_child(void);
static void wait_or_exit(pid_t first_child, pid_t second_child);
static void merge_and_write(FILE* first_child_output, FILE* second_child_output);
static void usage(void);
static void message_exit(char* message);

char *programm_name = "<not set yet>";		// stores the program name

/**
 * @brief sorts the input given in stdin alphabetically using merge sort
 * @details reads lines from stdin und parses them 1/2 to each of two children
 * recursivly. After the child processes report back with their sorted halfes
 * the parent process merges the results and prints them on stdout.<br>
 *
 * How it works:<br>
 *  - read input from stdin<br>
 *  - the first input always gets saved into first_line<br>
 *  - if no further input comes <br>
 *  	- then the process writes first_line to stdout<br>
 *  - else if other input comes<br>
 *  	- create 2 child processes, set up pipes for communication and exec<br>
 *  	- alternating forward the input from stdin to both children<br>
 *  	- after the input is completed send EOF to children and wait for their results<br>
 *  	- merge results from both children<br>
 *
 * @param argc number of arguments
 * @param argv arguments
 * @return EXIT_SUCCESS if process finished, otherwise (if any error in the process
 * or it's children appears) EXIT_FAILURE
*/
int main(int argc, char *argv[]) {
	// save program name and check synopsis
	programm_name = argv[0];
	if (argc != 1) {
		usage();
	}

	// define pipes for communication with childeren (2 pipes each)
	child_t child_1 = {0};
	child_t child_2 = {0};

	int counter = 0;
	int strlen = 0;
	size_t size = 0;
	char *line = NULL;	
	char *first_line = NULL;

	// read lines from input
	while ((strlen = getline(&line, &size, stdin)) != EOF) {
		counter++;
		switch (counter) {
			case 1:
				// if first line gets read, store it for later use
				first_line = store_line(line, strlen);
				break;
			case 2:
				// if a second line gets read, fork children and distribute lines between them
				child_1 = fork_child();
				child_2 = fork_child();

				fprintf(child_1.in, "%s", first_line);
				fprintf(child_2.in, "%s", line);
				free(first_line);
				break;
			default:
				// for every line after the second, send them to the children alternatingly
				if (counter % 2 != 0) {
					fprintf(child_1.in, "%s", line);
				} else {
					fprintf(child_2.in, "%s", line);
				}
		}
	}
	free(line);

	// send EOF the both children
	// the if is necessary, because in the last layer of children
	// child_1 and child_2 are NULL. Calling fclose() on NULL
	// results in problems
	if (child_1.in != NULL || child_2.in != NULL) {
		fclose(child_1.in);
		fclose(child_2.in);
	}
	
	// if the current process got only one input line, write it back to stdout (=to_parent pipe)
	if (counter == 1)  {
		fprintf(stdout, "%s", first_line);
		free(first_line);
		exit(EXIT_SUCCESS);
	}
	
	// wait for both children, then merge their results
	wait_or_exit(child_1.id, child_2.id);
	merge_and_write(child_1.out, child_2.out);

	// close last remaining pipe
	fclose(child_1.out);
	fclose(child_2.out);

	exit(EXIT_SUCCESS);
}

/**
 * @brief stores the value of line into a new char pointer
 * @details allocates memory and copies the value of line
 * into there.
 * @param line length of the line
 * @param length line to be stored
 * @return pointer to stored line
 */
static char *store_line(char* line, int length) {
	char *stored_line =  malloc((length + 1) * sizeof(char));
	if (stored_line == NULL) {
		message_exit("malloc failed for inputted line");
	}
	strcpy(stored_line, line);
	if (stored_line == NULL) {
		message_exit("strcpy failed to copy first line");
	}
	stored_line[length] = '\0';

	return stored_line;
}

/**
 * @brief forks a child process for the merge sort
 * @details forks the process. the child execs forksort again.
 * the parent continues and returns pipe file descriptors to and
 * from the child.
 * @return pipe file descriptors to and from the created chile and 
 * the childs' pid
 */
static child_t fork_child(void) {
	int pipefd_to_child[2];
	int pipefd_to_parent[2];
	pid_t pid_child;
	child_t child = {0};

	// open pipes to and from the future child
	if (pipe(pipefd_to_child) == -1) {
		message_exit("pipe() failed - " + errno);
	}
	if (pipe(pipefd_to_parent) == -1) {
		message_exit("pipe() failed - " + errno);
	};

	pid_child = fork();
	switch (pid_child) {
		case -1:
			message_exit("fork() failed");
		case 0: // in child
			// close to_child write and redirect to_child read to stdin
			if (close(pipefd_to_child[1]) == -1) {
				message_exit("close() failed for child pipe - " + errno);
			}
			if (dup2(pipefd_to_child[0], STDIN_FILENO) == -1) {
				message_exit("dup2() failed - " + errno);
			}

			// close to_parent read and redirect to_parent write to stdout
			if (close(pipefd_to_parent[0]) == -1) {
				message_exit("close() failed for child pipe - " + errno);
			};
			if (dup2(pipefd_to_parent[1], STDOUT_FILENO) == -1) {
				message_exit("dup2() failed - " + errno);
			}

			if (execl("forksort", "forksort", (char*) 0) == -1) {
				message_exit("exec failed to execute child process - " + errno);
			}
		default: // in parent
			// save child pid and open file descriptors
			child.id = pid_child;
			child.in = fdopen(pipefd_to_child[1], "w");
			child.out = fdopen(pipefd_to_parent[0], "r");
			if (child.in == NULL || child.out == NULL) {
				message_exit("failed to open file stream for childs' pipe");
			}

			// close to_child read and to_parent write
			close(pipefd_to_child[0]);
			close(pipefd_to_parent[1]);
	}

	return child;
}

/**
 * @brief waits for both children to finish
 * @details waits for both children to finish. If any error occures in
 * any of the children, print an error message and exit the process
 * @param first_child pid of the first child to wait on
 * @param second_child pid of the second child to wait on
 */
static void wait_or_exit(pid_t first_child, pid_t second_child) {
	int first_status, second_status;
	pid_t first_pid,second_pid;

	// wait for first child
	while((first_pid = waitpid(first_child, &first_status, 0) != first_child)) {
		if (first_pid != -1) continue;
		if (errno == EINTR) message_exit("waiting for chlid process was interruped");
		message_exit("encountered an error when waiting for child process - could not wait");
	}

	// wait for second child
	while((second_pid = waitpid(second_child, &second_status, 0) != second_child)) {
		if (second_pid != -1) continue;
		if (errno == EINTR) message_exit("waiting for chlid process was interruped");
		message_exit("encountered an error when waiting for child process - could not wait");
	}

	// if any child did not terminate successfuly termine process
	if (WEXITSTATUS(first_status) != EXIT_SUCCESS || WEXITSTATUS(second_status) != EXIT_SUCCESS) {
		message_exit("Child process failed to terminate successfully");
	}
}

/**
 * @brief merge results of both children and write them to stdout
 * @details compare the ouput of the children, sort them accordingly
 * and write the results alphabetically to stdout
 * @param first_child_output output stream of the first child
 * @param second_child_output output stream of the second child
 */
static void merge_and_write(FILE* first_child_output, FILE* second_child_output) {
	size_t first_current_size = 0;
	size_t second_current_size = 0;
	char *first_current_line = NULL;
	char *second_current_line = NULL;
	
	// read initial line
	int check_child_1 = getline(&first_current_line, &first_current_size, first_child_output);
	int check_child_2 = getline(&second_current_line, &second_current_size, second_child_output);
	
	// while both children have output
	while (check_child_1 != EOF && check_child_2 != EOF) {
		int cmp = strcmp(first_current_line, second_current_line);

		if (cmp < 0) {
			// process line from first child and read new output
			fprintf(stdout, "%s", first_current_line);
			check_child_1 = getline(&first_current_line, &first_current_size, first_child_output);
		} else if (cmp > 0) {
			// process line from second child and read new output
			fprintf(stdout, "%s", second_current_line);
			check_child_2 = getline(&second_current_line, &second_current_size, second_child_output);
		} else {
			// process lines from both children and read new output
			fprintf(stdout, "%s", first_current_line);
			check_child_1 = getline(&first_current_line, &first_current_size, first_child_output);
			fprintf(stdout, "%s", second_current_line);
			check_child_2 = getline(&second_current_line, &second_current_size, second_child_output);
		}
	}

	if (check_child_1 != EOF) {
		// if the second child has not output left, but the first one has: get all that is left
		fprintf(stdout, "%s", first_current_line);
		while ((check_child_1 = getline(&first_current_line, &first_current_size, first_child_output)) != EOF) {
			fprintf(stdout, "%s", first_current_line);
		}
	} else if (check_child_2 != EOF) { 
		// if the first child has not output left, but the second one has: get all that is left
		fprintf(stdout, "%s", second_current_line);
		while ((check_child_2 = getline(&second_current_line, &second_current_size, second_child_output)) != EOF) {
			fprintf(stdout, "%s", second_current_line);
		}
	}

	free(first_current_line);
	free(second_current_line);
}

/**
 * @brief prints the message and terminates the program
 * @details prints the provided message (inculding the program name) 
 * to stderr and terminates the programm with EXIT_FAILURE
 * @param message provided message to print (exculding the program name)
 */
static void message_exit(char* message) {
	fprintf(stderr, "[%s] %s\n", programm_name, message);
	exit(EXIT_FAILURE);
}

/**
 * @brief prints usage
 * @details prints usage (as specified in the exercise) to stderr
 */
static void usage(void) {
	fprintf(stderr,"Usage %s\n", programm_name);
	exit(EXIT_FAILURE);
}