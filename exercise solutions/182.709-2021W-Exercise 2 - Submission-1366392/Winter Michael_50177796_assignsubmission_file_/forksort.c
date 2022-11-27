/**									
    @author Michael Winter 01426162
    @brief  Recursive mergesort using pipes & forking
    @date   05.12.2021
	@tested on inf120
	@hint   adjust execlp path if needed
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

/**
* Program entry point.
* @param argc The argument counter
* @param argv The argument vector
* @return Returns EXIT_SUCCES on success, otherwise EXIT_FAILURE
*/
int main(int argc, char **argv) {

	/* if input >= 2 lines we create pipes and fork */
	char *arg_one = NULL, *arg_two = NULL;
    size_t bufsize = 0;
    ssize_t characters_one = getline(&arg_one, &bufsize, stdin);
    ssize_t characters_two = getline(&arg_two, &bufsize, stdin);

	if (characters_two <= 0) {
		printf("%s", arg_one);
		free(arg_one);
		free(arg_two);
		exit(EXIT_SUCCESS);
	} 

	/* Child0 */
    int pipefd_a[2], pipefd_b[2];
    pipe(pipefd_a);
    pipe(pipefd_b);

    int pid_x = fork();       
	if (pid_x < 0) {
        fprintf(stderr, "pid_x_fork_error!\n");
        exit(EXIT_FAILURE);
    } else if (pid_x == 0) {

		dup2(pipefd_a[0], STDIN_FILENO);
        dup2(pipefd_b[1], STDOUT_FILENO);
		close(pipefd_a[0]);
        close(pipefd_b[0]);
        close(pipefd_a[1]);
        close(pipefd_b[1]);

		// adjust path if needed
		execlp("/homes/y01426162/forksort", "forksort", NULL);
        fprintf(stderr, "should not be reached (child0)\n");
		exit(EXIT_FAILURE);

    } else {

		/* Child1 */
        int pipefd_c[2], pipefd_d[2];
        pipe(pipefd_c);
        pipe(pipefd_d);

        int pid_y = fork();     
        if (pid_y < 0) {
            fprintf(stderr, "pid_y_fork_error!\n");
            exit(EXIT_FAILURE);
        } else if (pid_y == 0) {

			dup2(pipefd_c[0], STDIN_FILENO);
            dup2(pipefd_d[1], STDOUT_FILENO);
			close(pipefd_a[0]);
            close(pipefd_b[0]);
            close(pipefd_c[0]);
            close(pipefd_d[0]);
			close(pipefd_a[1]);
            close(pipefd_b[1]);
            close(pipefd_c[1]);
            close(pipefd_d[1]);
		
			// adjust path if needed
			execlp("/homes/y01426162/forksort", "forksort", NULL);
            fprintf(stderr, "should not be reached (child1)\n");
			exit(EXIT_FAILURE);

		} else {
		
		/* Parent */
		FILE *pipe_a = fdopen(pipefd_a[1], "w");
		if (pipe_a == NULL)
			exit(EXIT_FAILURE);

		FILE *pipe_c = fdopen(pipefd_c[1], "w");
		if (pipe_c == NULL)
			exit(EXIT_FAILURE);

		fwrite(arg_one, characters_one, 1, pipe_a);
		fflush(pipe_a);
		fwrite(arg_two, characters_two, 1, pipe_c);
		fflush(pipe_c);

		free(arg_one);
		free(arg_two);

		/* read rest of input lines and write them to pipes */
		int toggle = 1;
		char *buffer = NULL;
  		ssize_t characters = getline(&buffer, &bufsize, stdin);

		while (characters >= 0) {
			if (toggle) {
				fwrite(buffer, characters, 1, pipe_a);
				fflush(pipe_a);
                toggle = 0;
            } else {
				fwrite(buffer, characters, 1, pipe_c);
				fflush(pipe_c);
                toggle = 1;
            }
			characters = getline(&buffer, &bufsize, stdin);
		}

		free(buffer);
		fclose(pipe_a);
		fclose(pipe_c);
		close(pipefd_a[0]);
        close(pipefd_c[0]);
        close(pipefd_a[1]);
		close(pipefd_b[1]);
        close(pipefd_c[1]);
		close(pipefd_d[1]);

		/* wait for child processes to terminate */        
		int status, a;
		a = wait(&status);

		while ((a != pid_x) && (a != pid_y)) {
			if (status == -1) 
				exit(EXIT_FAILURE);
			a = wait(&status);
		}

		/* read from pipes */
		char *one = NULL, *two = NULL; 
		int read_one = 1, read_two = 1;
		FILE *pipe_b = fdopen(pipefd_b[0], "r");
		FILE *pipe_d = fdopen(pipefd_d[0], "r");

		while (1) {
			if (read_one == 1)
				characters_one = getline(&one, &bufsize, pipe_b);
		
			if (read_two == 1)
				characters_two = getline(&two, &bufsize, pipe_d);

			if ((characters_one <= 0) && (characters_two <= 0))
				break;
	
			if (characters_one <= 0) {
				printf("%s", two);
				read_one = 0;
				read_two = 1;
				continue;		
			}

			if (characters_two <= 0) {
				printf("%s", one);
				read_one = 1;
				read_two = 0;
				continue;		
			}

			/* print the smaller value to stdout */
			if (strcmp(one, two) <= 0) {
				printf("%s", one);
				one = NULL; 
				read_one = 1;
				read_two = 0;
			} else {
				printf("%s", two);
				two = NULL; 
				read_one = 0;
				read_two = 1;
			}
		}

		free(one);
		free(two);
		fclose(pipe_b);
		fclose(pipe_d);		
		close(pipefd_b[0]);
		close(pipefd_d[0]);

		}
	}
	return EXIT_SUCCESS;
}

