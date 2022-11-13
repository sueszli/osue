/**
 * @file forkFFT.c
 * @author Andreas Himmler 11901924
 * @date 12.12.2021
 *
 * @brief Implements the Cooley-Tukey Fast Fourier Transform algorithm
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <complex.h>
#include <string.h>
#include <errno.h>
//#include <math.h>

#define PI 3.141592654 /**< Value of the constant PI */

static char *program; /**< Name of the program */

/**
 * @brief Structure that holds all information of a child. (pid and pipes)
 */
typedef struct child
{
	pid_t pid;
	int write[2];
	int read[2];
} child;


/**
 * @brief Prints the usage of a program
 * @details uses program
 */
static void
usage(void)
{
	fprintf(stderr, "[%s] Usage: %s\n", program, program);
	exit(EXIT_FAILURE);
}

/**
 * @brief Parses the string line and the float complex that the string represented.
 * @param line The string that should be parsed
 * @return float complex that represents the string line.
 */
static float complex
parse_complex(char *line)
{
	float r = 0.0;
	float i = 0.0;

	char *endp;
	r = strtof(line, &endp);
	i = strtof(endp, NULL);

	return r + i * I;
}

/**
 * @brief Forks the program
 * @details uses program
 * @param this the information of the child to be forked
 * @param other the information of the other child
 * @return 0 if this is the parent process
 * @return -1 if a error occured. (Also prints the error message to stderr)
 */
static int
fork_child(child this, child other)
{
	this.pid = fork();

	switch (this.pid)
	{
	case -1:
		fprintf(stderr, "[%s] %s\n", program, strerror(errno));
		return -1;
	case 0:
		// CHILD
		close(other.write[0]);
		close(other.write[1]);
		close(other.read[0]);
		close(other.read[1]);

		close(this.write[1]);
		close(this.read[0]);

		dup2(this.write[0], STDIN_FILENO); // TODO: ERROR
		dup2(this.read[1], STDOUT_FILENO); // TODO: ERROR

		close(this.write[0]);
		close(this.read[1]);

		execl(program, program, NULL);

		fprintf(stderr, "[%s] Could not execute child process!\n", program);
		return -1;
	default:
		// PARENT
		return 0;
	}
}

/**
 * @brief The main function. Manages the reading of the input, the passing of
 *        the lines to the children and reading their outputs.
 *        Also calculates the FFT.
 * @details uses program
 * @details uses PI
 * @param argc Number of arguments passed
 * @param argv The arguments
 * @return EXIT_FAILURE if a error occurs
 * @return EXIT_SUCCESS if no error occurs
 */
int
main(int argc, char* argv[])
{
	program = argv[0];

    // If there are arguments => EXIT
	if (argc != 1) usage();

	char *line1 = NULL;
	char *line2 = NULL;
	size_t line1_len = 0;
	size_t line2_len = 0;

    // If no line can be read => EXIT
	if (getline(&line1, &line1_len, stdin) == -1) {
		fprintf(stderr, "[%s] No line\n", program);
		free(line1);
		exit(EXIT_FAILURE);
	}

    // If length of first line is 1 => EXIT
	if (strlen(line1) == 1) {
		fprintf(stderr, "[%s] Empty line\n", program);
		free(line1);
		exit(EXIT_FAILURE);
	}

    // If only one line could be read => output this and exit, if more continue
	if (getline(&line2, &line2_len, stdin) == -1) {
		float complex z = parse_complex(line1);
		fprintf(stdout, "%f %f*i\n", creal(z), cimag(z));

		fprintf(stdout, "\n");
		fprintf(stdout, "%f\n", creal(z));
		free(line1);
		free(line2);
		exit(EXIT_SUCCESS);
	}

    // Initialize children:
	child even;
	child odd;
	even.pid = 0;
	odd.pid = 0;
    // Open pipes:
	if (pipe(even.write) == -1) {
        free(line1);
        free(line2);
        fprintf(stderr, "[%s] %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }
	if (pipe(even.read)) {
        free(line1);
        free(line2);
        fprintf(stderr, "[%s] %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }
	if (pipe(odd.write)) {
        free(line1);
        free(line2);
        fprintf(stderr, "[%s] %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }
	if (pipe(odd.read)) {
        free(line1);
        free(line2);
        fprintf(stderr, "[%s] %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Fork children:
	if (fork_child(even, odd) == -1) {
		free(line1);
		free(line2);
		close(even.write[0]);
		close(even.read[1]);
		close(odd.write[0]);
		close(odd.read[1]);
		exit(EXIT_FAILURE);
	}

	if (fork_child(odd, even) == -1) {
		free(line1);
		free(line2);
		close(even.write[0]);
		close(even.read[1]);
		close(odd.write[0]);
		close(odd.read[1]);
		exit(EXIT_FAILURE);
	}

	// PARENT: Start
    // Close pipes only used by the children:
	close(even.write[0]);
	close(even.read[1]);
	close(odd.write[0]);
	close(odd.read[1]);

    // Open Pipes to write to the children.
	FILE *write_even = fdopen(even.write[1], "w");
    if (write_even == NULL) {
        free(line1);
        free(line2);
        close(even.write[0]);
        close(even.read[1]);
        close(odd.write[0]);
        close(odd.read[1]);
        fprintf(stderr, "[%s] %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }
	FILE *write_odd = fdopen(odd.write[1], "w"); // TODO: ERROR
    if (write_even == NULL) {
        free(line1);
        free(line2);
        close(even.write[0]);
        close(even.read[1]);
        close(odd.write[0]);
        close(odd.read[1]);
        fflush(write_even);
        fclose(write_even);
        fprintf(stderr, "[%s] %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }

	int n = 0;

    // Pass input to the children.
	do
	{
		fprintf(write_even, "%s", line1);
		fprintf(write_odd, "%s", line2);
		n+=2;

		if(getline(&line1, &line1_len, stdin) == -1) break;
		if(getline(&line2, &line2_len, stdin) == -1) {
			free(line1);
			free(line2);
			fflush(write_even);
			fflush(write_odd);
			fclose(write_even);
			fclose(write_odd);
			fprintf(stderr, "[%s] Number of input values is not a power of 2!\n", program);
			exit(EXIT_FAILURE);
		}
	} while(1);

    // Free input lines:
	free(line1);
	free(line2);

    // Close Pipes:
	fflush(write_even);
	fflush(write_odd);
	fclose(write_even);
	fclose(write_odd);

	int status_even;
	int status_odd;

    // Wait for children:
	waitpid(even.pid, &status_even, 0);
	waitpid(odd.pid, &status_odd, 0);
	if(WEXITSTATUS(status_even) != EXIT_SUCCESS || WEXITSTATUS(status_odd) != EXIT_SUCCESS){
        fprintf(stderr, "[%s] Child process terminated with error.\n", program);
        exit(EXIT_FAILURE);
    }

	char *ret = NULL;
	size_t ret_len = 0;

    // Open Stream for reading outputs of the children:
	FILE *read_even = fdopen(even.read[0], "r");
    if (read_even == NULL) {
        close(even.write[0]);
        close(even.read[1]);
        close(odd.write[0]);
        close(odd.read[1]);
        fprintf(stderr, "[%s] %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }
	FILE *read_odd = fdopen(odd.read[0], "r");
    if (read_odd == NULL) {
        close(even.write[0]);
        close(even.read[1]);
        close(odd.write[0]);
        close(odd.read[1]);
        fprintf(stderr, "[%s] %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }

	float complex rE;
	float complex rO;
	int k = 0;
	float complex all[n];

	// Read calculated outputs from both children and calculate FFT:
	while (getline(&ret, &ret_len, read_even) != -1)
	{
		rE = parse_complex(ret);

        // If uneven number of lines => error
		if (getline(&ret, &ret_len, read_odd) == -1) {
			fclose(read_even);
			fclose(read_odd);
			free(ret);
			exit(EXIT_FAILURE);
		}

        // If child did not return a value => finished (Tree starts now)
		if (strlen(ret) <= 1) break;

		rO = parse_complex(ret);

        // Formula from the exercise:
		//complex float factor = cos(-2 * PI * k / n) + I * sin(-2 * PI * k / n);
		complex float factor = cexp(-2 * PI * k / n * I);
		complex float r1 = rE + factor * rO;
		complex float r2 = rE - factor * rO;

		all[k] = r1;
		all[k + n/2] = r2;

		k++;
	}
	free(ret);

    // Pass calculated values to parent.
	for(int i = 0; i < n; i++) fprintf(stdout, "%f %f*i\n", creal(all[i]), cimag(all[i]));

    // Indicate beginning of the tree
	fprintf(stdout, "\n");

    // Exit everything:
	fclose(read_even);
	fclose(read_odd);

	exit(EXIT_SUCCESS);
}
