/**
 * @file main.c
 * @author Yuhang Jiang <e12019854@student.tuwien.ac.at>
 * @brief Fast Fourier Transform with fork and pipes
 * @date 14.11.2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>

/**
 * Max number of branches = 2^nr of processes (Default: 12 = 4096 processes)
 */
#define MAX_BRANCHES 12

/**
 * Program name
 */
const char *PROG_NAME;

const float PI = 3.141592654f;

/**
 * Complex numbers with a real part and an imaginary part.
 */
struct complex_nr {
    float r;
    float i;
};

/**
 * Add 2 complex numbers.
 * @param number1
 * @param number2
 * @return Result
 */
static struct complex_nr complex_add(struct complex_nr *number1, struct complex_nr *number2)
{
    struct complex_nr newVal = {.r = number1->r + number2->r, .i = number1->i + number2->i};
    return newVal;
}

/**
 * Subtract 2 complex numbers.
 * @param number1
 * @param number2
 * @return Result
 */
static struct complex_nr complex_subt(struct complex_nr *number1, struct complex_nr *number2)
{
    struct complex_nr newVal = {.r = number1->r - number2->r, .i = number1->i - number2->i};
    return newVal;
}

/**
 * Multiply 2 complex numbers.
 * @param number1
 * @param number2
 * @return Result
 */
static struct complex_nr complex_mul(struct complex_nr *number1, struct complex_nr *number2)
{
    float a = number1->r;
    float b = number1->i;
    float c = number2->r;
    float d = number2->i;
    struct complex_nr newVal = {.r = a * c - b * d, .i = a * d + b * c};
    return newVal;
}

/**
 * Prints an error message and exits with EXIT_FAILURE.
 * @param func_name Name of the failed function.
 */
static void print_error(char *func_name)
{
    fprintf(stderr, "%d [%s] ERROR: %s failed: %s\n", getpid(), PROG_NAME, func_name, strerror(errno));
}

/**
 * Indicates an error value where errno cannot be used.
 */
static int error = 0;

/**
 * Parses the input string into a complex number
 * @param line_buf input string
 * @return complex_nr of the input values
 */
static struct complex_nr parse_nr(char *line_buf)
{
    char *endptr;

    float read_number_r = strtof(line_buf, &endptr);
    if (errno != 0) {
        print_error("strtof");
        struct complex_nr ret = {.r = 0, .i = 0};
        return ret;
    }
    // check that there is a number
    if (line_buf == endptr) {
        fprintf(stderr, "[%s] ERROR: Invalid input: %s"
                        "[%s] Inputs have to be this format: NUMBER [NUMBER]\n",
                PROG_NAME, line_buf, PROG_NAME);
        struct complex_nr ret = {.r = 0, .i = 0};
        error = 1;
        return ret;
    }

    float read_number_i = 0;

    // optional second number
    if (*endptr != '\n') {
        // Check for space between the 2 numbers
        if (*endptr != ' ') {
            fprintf(stderr, "[%s] ERROR: Invalid input: %s"
                            "[%s] Inputs have to be this format: NUMBER [NUMBER]\n",
                    PROG_NAME, line_buf, PROG_NAME);
            struct complex_nr ret = {.r = 0, .i = 0};
            error = 1;
            return ret;
        }
        char *endptr2;
        read_number_i = strtof(endptr, &endptr2);

        // check that there is a number and there isn't anything after it
        if (endptr2 == endptr || *endptr2 != '\n') {
            fprintf(stderr, "[%s] ERROR: Invalid input: %s"
                            "[%s] Inputs have to be this format: NUMBER [NUMBER]\n",
                    PROG_NAME, line_buf, PROG_NAME);
            struct complex_nr ret = {.r = 0, .i = 0};
            error = 1;
            return ret;
        }
        if (errno != 0) {
            print_error("strtof");
            struct complex_nr ret = {.r = 0, .i = 0};
            return ret;
        }
    }

    struct complex_nr val = {.r = read_number_r, .i = read_number_i};
    return val;
}

static FILE *stdin_c1 = NULL;   /**< STDIN for child 1 */
static FILE *stdin_c2 = NULL;   /**< STDIN for child 2 */
static FILE *stdout_c1 = NULL;  /**< STDOUT for child 1 */
static FILE *stdout_c2 = NULL;  /**< STDOUT for child 2 */
static pid_t c1pid = -1;        /**< Process id of child 1 */
static pid_t c2pid = -1;        /**< Process id of child 2 */

/**
 * Creates the child processes and initializes the pipes.
 * @param max_bran Branch number
 * @return 0 if successful, -1 otherwise
 */
static int create_child_processes(int max_bran)
{
    int pipefd1[2];     /* stdin for child 1 */
    if (pipe(pipefd1) == -1) {
        print_error("pipe");
        return -1;
    }

    int pipefd1_2[2];   /* stdout for child 1 */
    if (pipe(pipefd1_2) == -1) {
        close(pipefd1[0]);
        close(pipefd1[1]);
        print_error("pipe");
        return -1;
    }

    c1pid = fork();
    if (c1pid == -1) {
        close(pipefd1[0]);
        close(pipefd1[1]);
        close(pipefd1_2[0]);
        close(pipefd1_2[1]);
        print_error("fork");
        return -1;
    }

    if (c1pid == 0) {   /* Child process 1 */
        close(pipefd1[1]);
        close(pipefd1_2[0]);
        if (dup2(pipefd1[0], STDIN_FILENO) == -1) {
            close(pipefd1[0]);
            close(pipefd1_2[1]);
            print_error("dup2");
            return -1;
        }
        if (dup2(pipefd1_2[1], STDOUT_FILENO) == -1) {
            close(pipefd1[0]);
            close(pipefd1_2[1]);
            print_error("dup2");
            return -1;
        }
        close(pipefd1[0]);
        close(pipefd1_2[1]);
        char max_bran_new[7];
        snprintf(max_bran_new, sizeof max_bran_new, "%d", max_bran - 1);
        if (execl("./forkFFT", "forkFFT", max_bran_new, NULL) == -1) {
            print_error("execl");
            return -1;
        }
    }

    close(pipefd1[0]);
    close(pipefd1_2[1]);

    int pipefd2[2];     /* stdin for child 2 */
    if (pipe(pipefd2) == -1) {
        print_error("pipe");
        return -1;
    }

    int pipefd2_2[2];   /* stdout for child 2*/
    if (pipe(pipefd2_2) == -1) {
        print_error("pipe");
        return -1;
    }

    c2pid = fork();
    if (c2pid == -1) {
        print_error("fork");
        return -1;
    }

    if (c2pid == 0) {   /* Child process 2 */
        close(pipefd2[1]);
        close(pipefd2_2[0]);
        if (dup2(pipefd2[0], STDIN_FILENO) == -1) {
            close(pipefd2[0]);
            close(pipefd2_2[1]);
            print_error("dup2");
            return -1;
        }
        if (dup2(pipefd2_2[1], STDOUT_FILENO) == -1) {
            close(pipefd2[0]);
            close(pipefd2_2[1]);
            print_error("dup2");
            return -1;
        }
        close(pipefd2[0]);
        close(pipefd2_2[1]);
        char max_bran_new[7];
        snprintf(max_bran_new, sizeof max_bran_new, "%d", max_bran - 1);
        if (execl("./forkFFT", "forkFFT", max_bran_new, NULL) == -1) {
            print_error("execl");
            return -1;
        }
    }

    close(pipefd2[0]);
    close(pipefd2_2[1]);

    stdin_c1 = fdopen(pipefd1[1], "w");
    if (stdin_c1 == NULL) {
        print_error("fdopen");
        return -1;
    }

    stdin_c2 = fdopen(pipefd2[1], "w");
    if (stdin_c2 == NULL) {
        print_error("fdopen");
        return -1;
    }

    stdout_c1 = fdopen(pipefd1_2[0], "r");
    if (stdout_c1 == NULL) {
        print_error("fdopen");
        return -1;
    }

    stdout_c2 = fdopen(pipefd2_2[0], "r");
    if (stdout_c2 == NULL) {
        print_error("fdopen");
        return -1;
    }
    return 0;
}

/**
 * Entry function of the program. The program will be called multiple times. It calculates
 * the fast fourier transform by splitting the problem and then merging it together. Instead of recursion,
 * the program will call itself using fork and execl and write the parts into stdin. The result of the child
 * processes will be read back and merged. The parent process will monitor the child processes and will print
 * the result into stdout.
 * @param argc
 * @param argv Max number of branches. (Should not be set by the user, use MAX_BRANCHES instead.)
 * @return EXIT_SUCESS if the program runs as expected. EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[])
{
    int max_bran;
    if (argc == 1) {
        max_bran = MAX_BRANCHES;
    } else {
        max_bran = (int) strtol(argv[1], NULL, 10);
        if (max_bran == 0) {
            fprintf(stderr, "Max number of branches reached!\n");
            exit(EXIT_FAILURE);
        }
    }

    PROG_NAME = argv[0];

    struct complex_nr *numbers_odd = NULL;
    struct complex_nr *numbers_even = NULL;

    char *line_buf_even = NULL;
    char *line_buf_odd = NULL;

    int number_count = 0;

    size_t bufsize = 0;
    char *line_buf = NULL;
    ssize_t linelen;

    struct complex_nr first_nr;

    // first number
    while ((linelen = getline(&line_buf, &bufsize, stdin)) != -1) {
        if (linelen > 1) {
            number_count++;

            first_nr = parse_nr(line_buf);
            if (errno != 0 || error == 1) {
                goto cleanup;
            }
            break;
        }
    }

    if (errno != 0) {
        print_error("getline");
        goto cleanup;
    }

    // second number (create child processes and write first and second number to child processes)
    while ((linelen = getline(&line_buf, &bufsize, stdin)) != -1) {
        if (linelen > 1) {
            number_count++;

            struct complex_nr number = parse_nr(line_buf);
            if (errno != 0 || error == 1) {
                goto cleanup;
            }

            if (create_child_processes(max_bran) == -1) {
                goto cleanup;
            }

            fprintf(stdin_c1, "%f %f\n", first_nr.r, first_nr.i);
            fprintf(stdin_c2, "%f %f\n", number.r, number.i);
            break;
        }
    }

    if (errno != 0) {
        print_error("getline");
        goto cleanup;
    }

    // all other numbers
    while ((linelen = getline(&line_buf, &bufsize, stdin)) != -1) {
        if (linelen > 1) {
            number_count++;

            struct complex_nr number = parse_nr(line_buf);
            if (errno != 0 || error == 1) {
                goto cleanup;
            }

            if (number_count % 2 == 1) {
                fprintf(stdin_c1, "%f %f\n", number.r, number.i);
                if (errno != 0) {
                    print_error("fprintf to stdin_c1");
                    goto cleanup;
                }
            } else {
                fprintf(stdin_c2, "%f %f\n", number.r, number.i);
                if (errno != 0) {
                    print_error("fprintf to stdin_c2");
                    goto cleanup;
                }
            }
        }
    }

    if (errno != 0) {
        print_error("getline");
        goto cleanup;
    }

    if (number_count == 0) {
        free(line_buf);
        fprintf(stderr, "[%s] ERROR: No Input!\n", PROG_NAME);
        return EXIT_FAILURE;
    }

    if (number_count == 1) {
        printf("%f %f\n", first_nr.r, first_nr.i);
        if (errno != 0) {
            print_error("printf");
            goto cleanup;
        }
        free(line_buf);
        return EXIT_SUCCESS;
    }

    if (number_count % 2 != 0) {
        fprintf(stderr, "[%s] Input is not even!\n", PROG_NAME);
        error = 1;
        goto cleanup;
    }

    fclose(stdin_c1);
    stdin_c1 = NULL;
    fclose(stdin_c2);
    stdin_c2 = NULL;

    numbers_even = malloc((number_count / 2) * sizeof(struct complex_nr));
    int index = 0;
    bufsize = 0;

    while (getline(&line_buf_even, &bufsize, stdout_c1) != -1) {
        errno = 0;
        char *endptr;

        float read_number_r = strtof(line_buf_even, &endptr);
        if (errno != 0) {
            print_error("strtof");
            goto cleanup;
        }

        float read_number_i = strtof(endptr, NULL);
        if (errno != 0) {
            print_error("strtof");
            goto cleanup;
        }

        numbers_even[index].r = read_number_r;
        numbers_even[index].i = read_number_i;
        index++;
    }

    numbers_odd = malloc((number_count / 2) * sizeof(struct complex_nr));
    index = 0;
    bufsize = 0;

    while (getline(&line_buf_odd, &bufsize, stdout_c2) != -1) {
        errno = 0;
        char *endptr;
        float read_number_r = strtof(line_buf_odd, &endptr);
        if (errno != 0) {
            print_error("strtof");
            goto cleanup;
        }

        float read_number_i = strtof(endptr, NULL);
        if (errno != 0) {
            print_error("strtof");
            goto cleanup;
        }

        numbers_odd[index].r = read_number_r;
        numbers_odd[index].i = read_number_i;
        index++;
    }

    int wstatus;
    if (waitpid(c1pid, &wstatus, 0) == -1) {
        print_error("waitpid");
        goto cleanup;
    }
    if (WEXITSTATUS(wstatus) != EXIT_SUCCESS) {
        fprintf(stderr, "[%s] ERROR: child process did not terminate with EXIT_SUCCESS\n", PROG_NAME);
        error = 1;
        goto cleanup;
    }
    c1pid = -1;

    if (waitpid(c2pid, &wstatus, 0) == -1) {
        print_error("waitpid");
        goto cleanup;
    }
    if (WEXITSTATUS(wstatus) != EXIT_SUCCESS) {
        fprintf(stderr, "[%s] ERROR: child process did not terminate with EXIT_SUCCESS\n", PROG_NAME);
        error = 1;
        goto cleanup;
    }
    c2pid = -1;

    for (int k = 0; k < number_count / 2; ++k) {
        const float n = (float) number_count;
        struct complex_nr calc_part = {.r = cosf((-2.0f * PI * (float) k) / n), .i = sinf((-2.0f * PI * (float) k) / n)};
        struct complex_nr mul_part = complex_mul(&calc_part, &numbers_odd[k]);
        struct complex_nr result = complex_add(&numbers_even[k], &mul_part);
        printf("%f %f\n", result.r, result.i);
        if (errno != 0) {
            print_error("printf");
            goto cleanup;
        }
    }
    for (int k = 0; k < number_count / 2; ++k) {
        const float n = (float) number_count;
        struct complex_nr calc_part = {.r = cosf((-2.0f * PI * (float) k) / n), .i = sinf((-2.0f * PI * (float) k) / n)};
        struct complex_nr mul_part = complex_mul(&calc_part, &numbers_odd[k]);
        struct complex_nr result = complex_subt(&numbers_even[k], &mul_part);
        printf("%f %f\n", result.r, result.i);
        if (errno != 0) {
            print_error("printf");
            goto cleanup;
        }
    }

cleanup:
    free(line_buf);
    free(line_buf_odd);
    free(line_buf_even);
    free(numbers_even);
    free(numbers_odd);

    if (stdin_c1 != NULL) fclose(stdin_c1);
    if (stdin_c2 != NULL) fclose(stdin_c2);

    if (c1pid > 0) {
        waitpid(c1pid, NULL, 0);
    }
    if (c2pid > 0) {
        waitpid(c2pid, NULL, 0);
    }

    if (stdout_c1 != NULL) fclose(stdout_c1);
    if (stdout_c2 != NULL) fclose(stdout_c2);

    if (errno != 0 || error == 1) return EXIT_FAILURE;
}
