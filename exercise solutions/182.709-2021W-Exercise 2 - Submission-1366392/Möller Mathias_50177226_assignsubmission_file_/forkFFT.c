/**
 * @author Mathias Möller, 12019833
 * @date 08.12.2021
 * @brief recursive implementation of the Cooley-Tukey Fast Fourier Transform using child-processes
 */

#include "forkFFT.h"
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <math.h>

/**
 * @brief Prints the correct usage of the program and exits with error code EXIT_FAILURE
 *
 * @param program_name name of the executing program
 */
#define USAGE(program_name) \
    fprintf(stdout, "USAGE: %s\n", program_name); \
    exit(EXIT_FAILURE);

/**
 * @brief creates a new child-process and 2 unnamed pipes that can write to stdin and read from stdout of the child-process
 * recursively executes this program without arguments in the child-process
 *
 * @return info of the child-process, including its pid and the FILE-streams to write to its stdin and read from its stdout
 */
static struct child_info new_child(void);

/**
 * @brief converts a read input-line to a complex number
 *
 * @param line pointer to the input-line to convert
 * @param current_value pointer to the complex number to store the converted value in
 * @return 0 on success, -1 on failure
 */
static int convert_line(const char *line, struct complex_number *current_value);

int main(int argc, char **argv) {

    FILE *input = stdin;

    // expect absolutely no parameters or options
    if ((optind != 1) || (argc != 1)) {
        USAGE(argv[0]);
    }

    // buffer for a single line = a single input-value
    char line[MAX_LINE_LENGTH];

    // needed to cache the first value, since we only fork child-processes after the second read value
    // and then still need to send the first value to one of the child-processes
    struct complex_number previous_value;
    struct complex_number current_value;

    // data-structures to save the info of the created child-processes
    struct child_info even_child;
    struct child_info odd_child;

    // total number of values read
    int n;

    for (n = 0; (fgets(line, MAX_LINE_LENGTH, input)) != NULL; ++n) { // fgets returns NULL upon reaching EOF
        previous_value = current_value;

        if (convert_line(line, &current_value) == -1) {
            fprintf(stderr,
                    "Error while converting an input to a floating-point number. Please use . to separate decimal places and the format <real> <imaginary>[*]i to represent complex numbers!\n");
            USAGE(argv[0]);
        }

        if (n == 1) {
            // we have a sufficient number of input values to start child-processes
            fflush(stdout); // just to be sure, flush stdout to not duplicate any output from the parent process in the child process

            even_child = new_child();
            odd_child = new_child();

            // write the current value to the odd-child (current value has "index" 1)
            // write the previous value to the even-child (previous value has "index" 0)
            fprintf(even_child.input_write, IMAGINARY_NUMBER_FORMAT, previous_value.real, previous_value.imaginary);
            fprintf(odd_child.input_write, IMAGINARY_NUMBER_FORMAT, current_value.real, current_value.imaginary);

            // force flush of the write-buffers if needed
            if (FORCE_FLUSH) {
                if ((fflush(even_child.input_write) == EOF) || (fflush(odd_child.input_write) == EOF)) {
                    fprintf(stderr, "Error: force-flush failed!\n");
                    USAGE(argv[0]);
                }
            }
            continue;
        }

        if (n > 1) {
            // we have already read more than 2 values and therefore can safely assume that the child-processes have been created

            // send the value to the child according to the "index" of the value
            if (n % 2 == 0) {
                fprintf(even_child.input_write, IMAGINARY_NUMBER_FORMAT, current_value.real, current_value.imaginary);
                if (FORCE_FLUSH) {
                    if (fflush(even_child.input_write) == EOF) {
                        fprintf(stderr, "Error: force-flush failed!\n");
                        USAGE(argv[0]);
                    }
                }
            } else {
                fprintf(odd_child.input_write, IMAGINARY_NUMBER_FORMAT, current_value.real, current_value.imaginary);
                if (FORCE_FLUSH) {
                    if (fflush(odd_child.input_write) == EOF) {
                        fprintf(stderr, "Error: force-flush failed!\n");
                        USAGE(argv[0]);
                    }
                }
            }
        }
    }

    if (n == 1) {
        // only a single value was given, return this value as-is and exit

        fprintf(stdout, IMAGINARY_NUMBER_FORMAT, current_value.real, current_value.imaginary);
        exit(EXIT_SUCCESS);
    }

    if ((n == 0) || ((n % 2) == 1)) {
        fprintf(stderr, "You must supply a positive, even number of input-values\n");

        // any child-processes (should they exist) can be killed, we no longer need them
        if (odd_child.child_pid > 0) {
            kill(odd_child.child_pid, SIGKILL);
        }
        if (even_child.child_pid > 0) {
            kill(even_child.child_pid, SIGKILL);
        }
        USAGE(argv[0])
    }

    // n >= 2 and even

    // close the streams to send EOF to both child processes, making them terminate
    if (fclose(even_child.input_write) == EOF) {
        fprintf(stderr, "Error while closing pipe-stream: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (fclose(odd_child.input_write) == EOF) {
        fprintf(stderr, "Error while closing pipe-stream: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // wait for both children to terminate
    int exit_status_even;
    int exit_status_odd;
    waitpid(even_child.child_pid, &exit_status_even, 0);
    waitpid(odd_child.child_pid, &exit_status_odd, 0);

    if ((exit_status_even != EXIT_SUCCESS) || (exit_status_odd != EXIT_SUCCESS)) {
        fprintf(stderr, "child process did not terminate successfully, aborting...\n");
        exit(EXIT_FAILURE);
    }

    // buffers for a single line of both child-outputs
    char out_odd[MAX_LINE_LENGTH];
    char out_even[MAX_LINE_LENGTH];

    // buffer for calculating the second half of the output
    struct complex_number odd_times_result_buffer[n / 2]; // stores the results calculated by this process multiplied by the result of the odd-child
    struct complex_number even_buffer[n / 2]; // stores the results of the even-child

    int k;

    // "for each line of the even-child"
    for (k = 0; fgets(out_even, MAX_LINE_LENGTH, even_child.output_read) !=
                NULL; ++k) { // fgets returns NULL upon reaching EOF

        // "if a corresponding line of the odd-child exists..."
        if (fgets(out_odd, MAX_LINE_LENGTH, odd_child.output_read) != NULL) { // fgets returns NULL upon reaching EOF

            // convert odd-child's result to a complex number
            struct complex_number odd_result;
            if (convert_line(out_odd, &odd_result) == -1) {
                fprintf(stderr,
                        "Error while converting an input from a child process to a number.\n");
                USAGE(argv[0])
            }

            // convert even-child's result to a complex number
            struct complex_number even_result;
            if (convert_line(out_even, &even_result) == -1) {
                fprintf(stderr,
                        "Error while converting an input from a child process to a number.\n");
                USAGE(argv[0])
            }

            even_buffer[k] = even_result;

            // calculate own result, according to the specification in the assignment
            struct complex_number tmp_result;
            struct complex_number result;

            tmp_result.real = cos((((2 * PI) + 0.0) / n) * (k * (-1)));
            tmp_result.imaginary = sin((((2 * PI) + 0.0) / n) * (k * (-1)));

            result.real = tmp_result.real * odd_result.real - tmp_result.imaginary * odd_result.imaginary;
            result.imaginary = tmp_result.real * odd_result.imaginary + tmp_result.imaginary * odd_result.real;

            odd_times_result_buffer[k] = result;

            result.real += even_result.real;
            result.imaginary += even_result.imaginary;

            // output own result
            fprintf(stdout, IMAGINARY_NUMBER_FORMAT, result.real, result.imaginary);

            if (FORCE_FLUSH) {
                if (fflush(stdout) == EOF) {
                    fprintf(stderr, "Error: force-flush failed!\n");
                    USAGE(argv[0]);
                }
            }

        } else {
            // insufficient number of results returned by odd-child
            fprintf(stderr, "Error: expected result from child-process, but none (or not enough) was returned!\n");
            exit(EXIT_FAILURE);
        }
    }

    // check if correct number of results was returned by even-child
    if (k != (n / 2)) {
        fprintf(stderr, "Error: unexpected number of results from child-process!\n");
        exit(EXIT_FAILURE);
    }

    /*
     * this would be soooooo nice, but sadly doesn't work for streams that don't simply read from a file, we needed our own buffer
     */
//    rewind(even_child.output_read);
//    rewind(odd_child.output_read);

    // calculate second half of the results using the results of the first
    for (k = 0; k < n / 2; ++k) {
        struct complex_number result = even_buffer[k];
        result.real -= odd_times_result_buffer[k].real;
        result.imaginary -= odd_times_result_buffer[k].imaginary;

        fprintf(stdout, IMAGINARY_NUMBER_FORMAT, result.real, result.imaginary);

        if (FORCE_FLUSH) {
            if (fflush(stdout) == EOF) {
                fprintf(stderr, "Error: force-flush failed!\n");
                USAGE(argv[0]);
            }
        }
    }

    // nothing failed, success!
    exit(EXIT_SUCCESS);
}
/**
 * @brief converts a read input-line to a complex number
 *
 * @param line pointer to the input-line to convert
 * @param current_value pointer to the complex number to store the converted value in
 * @return 0 on success, -1 on failure
 */
static int convert_line(const char *line, struct complex_number *current_value) {
    char *end = NULL;

    // reset current value
    (*current_value).real = 0;
    (*current_value).imaginary = 0;

    (*current_value).real = strtof(line, &end);

    if (*end != '\n') {
        (*current_value).imaginary = strtof(end, &end);

        // only accept ends "*i" or "i" for imaginary part of complex number
        if ((end[0] != 'i') && ((end[0] != '*') || (end[1] != 'i'))) {
            return -1;
        }
        *end = '\n';
    }

    if (*end != '\n') {
        return -1;
    }

    return 0;
}

/**
 * @brief creates a new child-process and 2 unnamed pipes that can write to stdin and read from stdout of the child-process
 * recursively executes this program without arguments in the child-process
 *
 * @return info of the child-process, including its pid and the FILE-streams to write to its stdin and read from its stdout
 */
static struct child_info new_child(void) {
    int pipe_input_fd[2];
    int pipe_output_fd[2];

    pipe(pipe_input_fd);
    if ((pipe_input_fd[0] == -1) || (pipe_input_fd[1] == -1)) {
        fprintf(stderr, "creation of pipe failed!\n");
        exit(EXIT_FAILURE);
    }

    pipe(pipe_output_fd);
    if ((pipe_output_fd[0] == -1) || (pipe_output_fd[1] == -1)) {
        fprintf(stderr, "creation of pipe failed!\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    struct child_info child;
    switch (pid) {
        case -1:
            fprintf(stderr, "Could not create a child process!\n");
            exit(EXIT_FAILURE);
        case 0:
            // inside child-process

            close(pipe_input_fd[1]); // close write-end of input-pipe
            close(pipe_output_fd[0]); // close read-end of output-pipe
            dup2(pipe_input_fd[0], STDIN_FILENO); // map read-end of input-pipe to stdin
            dup2(pipe_output_fd[1], STDOUT_FILENO); // map write-end of output-pipe to stdout
            close(pipe_input_fd[0]); // close read-end of input-pipe, since now mapped to stdin
            close(pipe_output_fd[1]); // close write-end of output-pipe, since now mapped to stdout

            execlp("./forkFFT", "./forkFFT", NULL);
            break;
        default:
            // inside parent-process, pid is the pid of the child-process
            close(pipe_input_fd[0]); // close read-end of input-pipe
            close(pipe_output_fd[1]); // close write-end of output-pipe
            child.child_pid = pid;
            child.input_write = fdopen(pipe_input_fd[1], "w");
            child.output_read = fdopen(pipe_output_fd[0], "r");

            if ((child.input_write == NULL) || (child.output_read == NULL)) {
                fprintf(stderr, "Something went wrong when connecting the pipes from parent to child process!\n");
                exit(EXIT_FAILURE);
            }

            break;
    }
    return child;
}
