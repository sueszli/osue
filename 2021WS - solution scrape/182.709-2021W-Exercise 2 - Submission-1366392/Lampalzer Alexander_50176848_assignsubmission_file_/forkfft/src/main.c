/**
 * forkFFT: Calculate Fourier Transformation using with forked processes
 * @author: Alexander Lampalzer <e12023145@student.tuwien.ac.at>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <argz.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>

/**
 * Default size of buffer, for holding complex numbers
 */
#define ARRAY_SIZE 16
#define ARRAY_ITEM COMPLEX

/**
 * Type definition of a complex number
 */
typedef struct COMPLEX {
    /**
     * Real part of complex number
     */
    float real;

    /**
     * Complex part of complex number
     */
    float i;
} COMPLEX;

/**
 * @brief Multiples two complex numbers
 * @details
 *  Two complex numbers can be multiplied according to the formula: (a + i*b)(c + i*d) = ac − bd + i*(ad + bc)
 *
 * @param a multiplier
 * @param b multiplicand
 * @return product
 */
COMPLEX mult_complex(COMPLEX a, COMPLEX b) {
    COMPLEX result;
    result.real = a.real * b.real - a.i * b.i;
    result.i = a.real * b.i + a.i * b.real;

    return result;
}

/**
 * @brief Sum of two complex numbers
 *
 * @param a term 1
 * @param b term 2
 * @return sum
 */
COMPLEX add_complex(COMPLEX a, COMPLEX b) {
    COMPLEX result;
    result.real = a.real + b.real;
    result.i = a.i + b.i;

    return result;
}

/**
 * @brief Subtract complex number b from a
 * @param a Term 1
 * @param b Term 2
 * @return Difference
 */
COMPLEX sub_complex(COMPLEX a, COMPLEX b) {
    COMPLEX result;
    result.real = a.real - b.real;
    result.i = a.i - b.i;

    return result;
}

/**
 * @brief Read a line or until EOF from a file descriptor
 *
 * @details
 *  Reads character by character from a file descriptor.
 *  Internally, the string is stored in a buffer, that is expanded on demand.
 *  The resulting string will be null terminated.
 *
 *  @attention do free(line) on success
 *
 * @param pname process name
 * @param fd file descriptor
 * @param line Result: Pointer to line
 * @param length Result: Length of string
 * @return 0 on success, -1 otherwise
 */
int read_line(char* pname, int fd, char **line, size_t *length) {
    char *buffer = malloc(BUFSIZ);
    size_t buffer_size = BUFSIZ;
    size_t buffer_items = 0;

    while (1 == 1) {
        if (buffer_items == buffer_size) { // Buffer expansion
            buffer = realloc(buffer, buffer_size * 2);
            buffer_size *= 2;

            if (buffer == NULL) {
                fprintf(stderr, "[%s] Error increasing line buffer: %s\n", pname, strerror(errno));
                return -1;
            }
        }

        ssize_t bytes_read = read(fd, &buffer[buffer_items], 1);
        buffer_items += bytes_read;

        if (bytes_read == -1) {
            fprintf(stderr, "[%s] Error reading input: %s\n", pname, strerror(errno));
            free(buffer);

            return -1;
        }

        if (bytes_read == 0 || buffer[buffer_items - 1] == '\n') { // No new data
            break;
        }
    }

    if (buffer_items > 0) {
        buffer_items += 1;
        buffer = realloc(buffer, buffer_items);

        if (buffer == NULL) {
            fprintf(stderr, "[%s] Error shrinking line buffer: %s\n", pname, strerror(errno));
            return -1;
        }

        buffer[buffer_items - 1] = '\0';

        *length = buffer_items;
        *line = buffer;

        return 0;
    } else {
        *length = buffer_items;
        *line = NULL;

        free(buffer);

        return 0;
    }
}

/**
 * @brief Converts a string with format "%f [%f]" to a complex number
 * @details
 *  strtof is applied two times to the input string.
 *  Also checks for a proper format in the string.
 *
 * @param pname process name
 * @param result Result: Complex number read from string
 * @param line Pointer to line
 * @return 0 on success
 */
int convert_line(char* pname, char *line, COMPLEX *result) {
    COMPLEX number;
    number.real = 0;
    number.i = 0;

    int component = 0;
    char *endptr = line;

    while (component < 2) {
        float converted = strtof(endptr, &endptr);

        if (errno != 0) {
            fprintf(stderr, "[%s] Failed to convert line: %s\n", pname, strerror(errno));
            return -1;
        }

        if (endptr == line) {
            fprintf(stderr, "[%s] No digits found in line: %s\n", pname, line);
            return -1;
        }

        if (*endptr != ' ' && *endptr != '\n') {
            fprintf(stderr, "[%s] Line contains invalid characters\n", pname);
            return -1;
        }


        if (component == 0) {
            number.real = converted;
        } else {
            number.i = converted;
        }

        component += 1;
    }

    if (*endptr != '\n' && *endptr != '\0') {
        fprintf(stderr, "[%s] Extra characters after second number\n", pname);

        return -1;
    }

    *result = number;

    return 0;
}

/**
 * @brief Reads the array from fd, as specified
 * @details
 * Reads line-by-line from fd
 * Tries to convert that line using convert_line
 * Returns the accumalated result of all lines in array and array_size
 *
 * @attention: free(array) on success
 *
 * @param pname process name
 * @param fd file descriptor
 * @param array Result: Array
 * @param array_size Result: Array size
 * @return 0 on success
 */
int read_into_array(char *pname, int fd, ARRAY_ITEM** array, size_t *array_size) {
    ARRAY_ITEM *buffer = calloc(ARRAY_SIZE, sizeof(ARRAY_ITEM));
    if (buffer == NULL) {
        fprintf(stderr, "[%s] Failed to allocate array buffer: %s\n", pname, strerror(errno));
        return -1;
    }

    size_t buffer_size = ARRAY_SIZE;
    size_t buffer_items = 0;

    while (1 == 1) {
        if (buffer_items == buffer_size) {
            buffer = reallocarray(buffer, buffer_items * 2, sizeof(ARRAY_ITEM));
            buffer_items *= 2;

            if (buffer == NULL) {
                fprintf(stderr, "[%s] Failed to increase array buffer: %s\n", pname, strerror(errno));
                return -1;
            }
        }

        char *line;
        size_t line_length;

        int read_line_result = read_line(pname, fd, &line, &line_length);
        if (read_line_result == -1) {
            free(buffer);
            return -1;
        }

        if (line_length == 0) {
            free(line);
            break;
        }

        COMPLEX number;
        int convert_line_result = convert_line(pname, line, &number);
        if (convert_line_result != 0) {
            free(buffer);
            free(line);
            return -1;
        }

        buffer[buffer_items] = number;
        buffer_items += 1;
        free(line);
    }


    *array_size = buffer_items;
    *array = buffer;

    return 0;
}

/**
 * @brief Forks the current process, opens two new pipes and remaps stdin / stdout
 * @details
 *  The current process if forked and two pipes are created.
 *  stdin is remapped and the new fd is returned in the parameter "in"
 *  stdout is remapped onto "out"
 *
 *  Using the call "write", one can (and should) write to the in fd.
 *
 *  After remapping, the current process is restarted from main.
 *
 * @param pname process name
 * @param in Result: write end of stdin of child
 * @param out Result: read end of stdout of child
 * @param child_pid Result: pid of the newly create child
 *
 * @return 0 on success
 */
int handle_fork(char *pname, int *in, int *out, pid_t *child_pid) {
    int fd_in[2];
    int fd_out[2];

    if (pipe(fd_in) == -1 || pipe(fd_out) == -1) {
        fprintf(stderr, "[%s] Failed to open pipe: %s\n", pname, strerror(errno));
        return -1;
    }

    *child_pid = fork();
    if (*child_pid == -1) {
        fprintf(stderr, "[%s] Failed to open pipe: %s\n", pname, strerror(errno));
        return -1;
    }

    if (*child_pid == 0) {
        close(fd_in[1]);
        close(fd_out[0]);

        dup2(fd_in[0], STDIN_FILENO); // Parent -> Child
        dup2(fd_out[1], STDOUT_FILENO); // Child -> Parent

        execlp(pname, pname, NULL);
    } else {
        close(fd_in[0]);
        close(fd_out[1]);
    }

    *in = fd_in[1];
    *out = fd_out[0];

    return 0;
}

/**
 * @brief Writes the contents of the array to the fd
 * @details
 *  Items from the array are written to the file descriptor.
 *  Items are seperated using a new line.
 *  After write has finished or errored, the fd is closed.
 *
 * @param array Array of items to write
 * @param array_size Number of items to write
 * @param fd file descriptor to write to
 *
 * @return 0 on success
 */
int write_array(ARRAY_ITEM *array, size_t array_size, int fd) {
    char str[array_size * 32]; // TODO: Dynamic size

    for (size_t i = 0; i < array_size; i++) {
        int len;
        len = sprintf(str, "%f %f\n", array[i].real, array[i].i);

        ssize_t nbytes = write(fd, str, len);
        if (nbytes == -1) {
            close(fd);
            return -1;
        }
    }

    close(fd);

    return 0;
}

/**
 * @brief Reads array from stdin, forks (if needed) and writes results to stdout
 * @details
 *  This is the main component of the program.
 *  1. The array is read from stdint
 *  2. If needed, the program is forked
 *  3. The array halves are written to the two children
 *  4. The result of the children is processed
 *  5. Using the result of the children, calculate own result
 *  6. Write result to stdout
 *
 * @param pname process name
 */
int read_and_fork(char *pname) {
    size_t array_size = 0;
    ARRAY_ITEM *array;
    int read_array_result = read_into_array(pname, STDIN_FILENO, &array, &array_size );
    if (read_array_result == -1) {
        // Error already printed in function
        exit(EXIT_FAILURE);
    }

    if (array_size == 0) {
        fprintf(stderr, "[%s] Array empty\n", pname);
        free(array);

        exit(EXIT_FAILURE);
    }

    if (array_size == 1) {
        fprintf(stdout, "%f %f\n", array[0].real, array[0].i);
        fflush(stdout);
        close(STDOUT_FILENO);
        free(array);

        exit(EXIT_SUCCESS);
    }

    if (array_size % 2 != 0) {
        fprintf(stderr, "[%s] Array size not even\n", pname);
        free(array);

        exit(EXIT_FAILURE);
    }

    ARRAY_ITEM *array_even = calloc(array_size / 2, sizeof(ARRAY_ITEM));
    ARRAY_ITEM *array_odd = calloc(array_size / 2, sizeof(ARRAY_ITEM));

    for (size_t i = 0; i < array_size / 2; i++) {
        array_even[i] = array[i * 2];
        array_odd[i] = array[i * 2 + 1];
    }

    int in_even;
    int out_even;
    pid_t pid_even;
    int err_even = handle_fork(pname, &in_even, &out_even, &pid_even);

    int in_odd;
    int out_odd;
    pid_t pid_odd;
    int err_odd = handle_fork(pname, &in_odd, &out_odd, &pid_odd);

    if (err_even == -1 || err_odd == -1) {
        free(array);
        free(array_even);
        free(array_odd);

        exit(EXIT_FAILURE);
    }

    int write_result_even = write_array(array_even, array_size / 2, in_even);
    int write_result_odd = write_array(array_odd, array_size / 2, in_odd);

    if (write_result_even == -1 || write_result_odd == -1) {
        fprintf(stderr, "[%s] Failed to write result: %s\n", pname, strerror(errno));

        free(array);
        free(array_even);
        free(array_odd);

        exit(EXIT_FAILURE);
    }

    size_t array_size_even;
    ARRAY_ITEM *result_even;
    int read_array_result_even = read_into_array(pname, out_even, &result_even, &array_size_even);
    size_t array_size_odd;
    ARRAY_ITEM *result_odd;
    int read_array_result_odd = read_into_array(pname, out_odd, &result_odd, &array_size_odd);

    if (read_array_result_even == -1 || read_array_result_odd == -1) {
        free(array);
        free(array_even);
        free(array_odd);
        free(result_even);
        free(result_odd);

        exit(EXIT_FAILURE);
    }


    int wstatus_even;
    pid_t exit_code_even = waitpid(pid_even, &wstatus_even, WUNTRACED | WCONTINUED);

    int wstatus_odd;
    pid_t exit_code_odd = waitpid(pid_odd, &wstatus_odd, WUNTRACED | WCONTINUED);

    if (wstatus_even == -1 || wstatus_odd == -1) {
        fprintf(stderr, "[%s] Child exited abnormally\n", pname);

        free(array);
        free(array_even);
        free(array_odd);
        free(result_even);
        free(result_odd);

        exit(EXIT_FAILURE);
    }

    if (exit_code_even == -1 || exit_code_odd == -1) {
        fprintf(stderr, "[%s] Error in waitpid: %s\n", pname, strerror(errno));

        free(array);
        free(array_even);
        free(array_odd);
        free(result_even);
        free(result_odd);

        exit(EXIT_FAILURE);
    }

    ARRAY_ITEM result[array_size];

    for (size_t k = 0; k < array_size / 2; k++) {
        size_t n = array_size;

        COMPLEX number;
        number.real = cos((-2 * M_PI * k) / n);
        number.i = sin((-2 * M_PI * k) / n);

        number = mult_complex(number, result_odd[k]);

        result[k] = add_complex(result_even[k], number);
        result[k + n / 2] = sub_complex(result_even[k], number);
    }

    int write_array_result = write_array(result, array_size, STDOUT_FILENO);

    free(array);
    free(array_even);
    free(array_odd);
    free(result_even);
    free(result_odd);

    if (write_array_result == -1) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}

int main(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "[%s] Unexpected parameters\n", argv[0]);
        return EXIT_FAILURE;
    }

    read_and_fork(argv[0]);

    return 0;
}
