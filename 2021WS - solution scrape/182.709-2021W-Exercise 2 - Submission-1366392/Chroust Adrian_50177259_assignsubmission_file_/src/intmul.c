/**
 * @file intmul.c
 * @author Adrian Chroust <e12023146@student.tuwien.ac.at>
 * @date 4.12.2021
 * @brief Multiply hexadecimal integers exceeding the limit of the data structure.
 * @details Add together to integers recursively with forks using this formula:
 *          A * B = A_h * B_h * 16^n + A_h * B_l * 16^(n/2) + A_l * B_h * 16^(n/2) + A_l * B_l
 *          where n is the number of digits of A and B (they have to have the same number of digits).
 *          If A and B are both of length 1, then they can be multiplied directly, otherwise they are calculated recursively.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * @brief The maximal allowed length of digits in the input integers.
 */
#define INTMUL_MAX_SAFE_LENGTH 128

/**
 * @brief The length to add when using the function while reading characters with the function read_line.
 */
#define LINE_READ_MEMORY_INCREASE_LENGTH 10

/**
 * @brief The offset of for the fork pipe up read pointer stored in a fork pipe array.
 */
#define FORK_PIPE_UP_READ 0
/**
 * @brief The offset of for the fork pipe up write pointer stored in a fork pipe array.
 */
#define FORK_PIPE_UP_WRITE 1
/**
 * @brief The offset of for the fork pipe down read pointer stored in a fork pipe array.
 */
#define FORK_PIPE_DOWN_READ 2
/**
 * @brief The offset of for the fork pipe down write pointer stored in a fork pipe array.
 */
#define FORK_PIPE_DOWN_WRITE 3

/**
 * @brief The program name detected by the main function.
 * @details Required for printing out the proper usage of the program through the usage function.
 * */
static char *prog_name;

/**
 * @brief This function prints the expected input parameters of the program to stderr.
 * @details The function is usually called when a call with unexpected inputs is detected.
 */
static void usage(void) {
    fprintf(stderr, "[%s] Usage: %s\n", prog_name, prog_name);
    fprintf(stderr, "     Input has to be two hexadecimal integers, each one on a single line\n");
    fprintf(stderr, "     Integers have to have the same number of digits, which is a power of 2\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief Reads the next non-empty line from stdin into dynamic memory and returns its result.
 * @details If the EOF is encountered without having started reading a line,
 *          then the line_return_length is set to 0 and dynamic memory holding a single \0 is returned.
 * @param line_return_length writes the length of the returned line into the pointer location.
 * @return If the operation was successful, the pointer to the line is returned, NULL otherwise.
 */
static char *read_line(unsigned int *line_return_length) {
    unsigned int memory_length = LINE_READ_MEMORY_INCREASE_LENGTH;
    char *line = malloc(memory_length * sizeof(char));
    if (line == NULL) return NULL;

    unsigned int l = 0;

    while (true) {
        int status = fgetc(stdin);
        char c = (char) status;

        // If the EOF is reached or a new line after having read at least one digit, the function prepares for return.
        if (status == EOF || (l > 0 && (c == '\r' || c == '\n'))) {
            line[l] = '\0';
            unsigned int reduced_memory_length = l + 1;
            // Reduce the size of the dynamic memory to only what is necessary.
            if (reduced_memory_length < memory_length) {
                char *reduced_line = realloc(line, reduced_memory_length * sizeof(char));
                if (reduced_line != NULL) line = reduced_line;
            }
            *line_return_length = l;
            return line;
        }

        // Write the current char into the dynamic memory.
        line[l] = c;
        l++;

        // If the next char including the \0 symbol would not fit anymore,
        // the dynamic memory has to be increased.
        if (l + 1 == memory_length) {
            memory_length += LINE_READ_MEMORY_INCREASE_LENGTH;
            char *increased_line = realloc(line, memory_length * sizeof(char));
            if (increased_line == NULL) {
                free(line);
                return NULL;
            }

            line = increased_line;
        }
    }
}

/**
 * @brief Checks if a string is a valid hexadecimal integer.
 * @details Accepts, if given, leading + and - as well as 0x at the start of the digits.
 * @param number The string to be checked.
 * @param length The length of the string excluding the \0.
 * @param digit_length The length of the digits, which are part of the string, is written to the pointer location of this parameter.
 * @return true if the string given is a valid hexadecimal integer, false otherwise.
 */
static bool is_valid_hex_int(char *number, unsigned int length, unsigned int *digit_length) {
    // Empty strings are not accepted.
    if (length == 0) return false;

    // Check for leading + and -.
    if (number[0] == '-' || number[0] == '+') {
        number++;
        length--;
    }

    // Checks for leading 0x.
    if (length > 2 && number[0] == '0' && tolower(number[1]) == 'x') {
        number += 2;
        length -= 2;
    }

    // Checks each remaining character if it is between [0-9] or [a-f]
    for (unsigned int i = 0; i < length; i++) {
        char c = (char) tolower(number[i]);

        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }

    *digit_length = length;
    return true;
}

/**
 * @brief Normalizes the integer by setting its starting pointer to the first non-zero digit to be read, the digits are also changed to lowercase.
 * @param return_number The new starting pointer of the number is written to the pointer location of this parameter.
 * @param return_length The new length of the number is written to the pointer location of this parameter.
 * @return true if the number is negative, false if it is positive.
 */
static bool normalize_positive_hex_int(char **return_number, unsigned int *return_length) {
    char pre = (*return_number)[0];
    bool negative = pre == '-';

    // Skip the prefix.
    if (negative || pre == '+') {
        (*return_number)++;
        (*return_length)--;
    }

    if (*return_length < 2) return negative;

    // Skip 0x.
    if (tolower((*return_number)[1]) == 'x') {
        *return_number += 2;
        *return_length -= 2;
    }

    // Skip all leading zeros as long as the number of digits is greater than 1.
    while (*return_length > 1 && (*return_number)[0] == '0') {
        (*return_number)++;
        (*return_length)--;
    }

    // Make all digits lowercase.
    for (unsigned int i = 0; i < *return_length; i++) {
        (*return_number)[i] = (char) tolower((*return_number)[i]);
    }

    return negative;
}

/**
 * @brief Writes a string to a file in reverse.
 * @param fd The file descriptor to be written to.
 * @param str The string to be written.
 * @param length The length of the string.
 * @return 0 on successful write, -1 otherwise.
 */
static int write_reverse(int fd, const char *str, unsigned int length) {
    for (unsigned int i = length; i > 0; i--) {
        char c = str[i - 1];
        if (write(fd, &c, sizeof(char)) != 1) return -1;
    }
    return 0;
}

/**
 * @details Parses a single hex digit as an integer.
 * @param digit The digit to be parsed.
 * @return A long integer containing the value of the digit.
 */
static long parse_hex_digit(char digit) {
    char str[2] = {digit, '\0'};
    return strtol(str, NULL, 16);
}

/**
 * @brief Opens two pipes, each one containing read and write file descriptors.
 * @param p An array, where the pipes should be stored. Has to have a length of at least 4.
 * @return 0 if opening the pipes was successful, -1 otherwise.
 */
static int open_fork_pipes(int *p) {
    // All not open pipes are set to -1.
    p[0] = -1;
    p[1] = -1;
    p[2] = -1;
    p[3] = -1;

    if (pipe(p) == -1) return -1;

    if (pipe(p + 2) == -1) {
        close(p[0]);
        close(p[1]);
        return -1;
    }

    return 0;
}

/**
 * @brief Close a single fork pipe direction, if it is not already closed.
 * @param d The pointer of the fork pipe to be closed.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int close_fork_pipe_direction(int *d) {
    // if the value of d is -1, then the pipe direction has already been closed.
    if (*d == -1) return 0;
    if (close(*d) != 0) return -1;
    *d = -1;
    return 0;
}

/**
 * @brief Close all open fork pipes, that are not already closed.
 * @param p The array, where the pipes are stored. Has to have a length of at least 4.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int close_fork_pipes(int *p) {
    return close_fork_pipe_direction(p + FORK_PIPE_UP_READ)
         | close_fork_pipe_direction(p + FORK_PIPE_DOWN_READ)
         | close_fork_pipe_direction(p + FORK_PIPE_UP_WRITE)
         | close_fork_pipe_direction(p + FORK_PIPE_DOWN_WRITE);
}

/**
 * @brief Redirects the fork pipes given by the parent to stdin and stdout and closes all pipes.
 * @param p The array, where the pipes are stored. Has to have a length of at least 4.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int redirect_parent_fork_pipes(int *p) {
    int *p_down_read = p + FORK_PIPE_DOWN_READ;
    int *p_up_write = p + FORK_PIPE_UP_WRITE;

    if (dup2(*p_down_read, STDIN_FILENO) == -1 || dup2(*p_up_write, STDOUT_FILENO) == -1) {
        close_fork_pipes(p);
        return -1;
    }

    if (close_fork_pipes(p) == -1) return -1;

    return 0;
}

/**
 * @brief Closes all fork pipes that are only to be used by the child process.
 * @param p The array, where the pipes are stored. Has to have a length of at least 4.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int close_unused_child_fork_pipes(int *p) {
    int *p_up_write = p + FORK_PIPE_UP_WRITE;
    int *p_down_read = p + FORK_PIPE_DOWN_READ;

    if (close_fork_pipe_direction(p_up_write) == -1 || close_fork_pipe_direction(p_down_read) == -1) {
        close_fork_pipes(p);
        return -1;
    }

    return 0;
}

/**
 * @brief Prints a hexadecimal number to stdout in reverse.
 * @param number The number to be printed.
 */
static void print_hex_reverse(long number) {
    if (number == 0) printf("0");
    else while (number > 0) {
        printf("%lx", number % 16);
        number /= 16;
    }
}

/**
 * @brief Shortcut for quickly closing three different children fork pipes.
 * @param p The array, where the pipes of the first child are stored. Has to have a length of at least 4.
 * @param q The array, where the pipes of the second child are stored. Has to have a length of at least 4.
 * @param r The array, where the pipes of the third child are stored. Has to have a length of at least 4.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int close_3_sibling_fork_pipes(int *p, int *q, int *r) {
    return close_fork_pipes(p) | close_fork_pipes(q) | close_fork_pipes(r);
}

/**
 * @brief Shortcut for quickly closing three different children fork pipes.
 * @param p The array, where the pipes of the first child are stored. Has to have a length of at least 4.
 * @param q The array, where the pipes of the second child are stored. Has to have a length of at least 4.
 * @param r The array, where the pipes of the third child are stored. Has to have a length of at least 4.
 * @param s The array, where the pipes of the fourth child are stored. Has to have a length of at least 4.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int close_4_sibling_fork_pipes(int *p, int *q, int *r, int *s) {
    return close_3_sibling_fork_pipes(p, q, r) | close_fork_pipes(s);
}

/**
 * @brief Pass the reversed integers a and b read from stdin to the child process pipes.
 * @details The numbers are split like this A_h, A_l, B_h, B_l, where "h" and "l" mean the upper and lower digits of a and b. The combinations are then passed down to the pipes.
 *          The numbers a and b are on the same line in stdin. To tell them apart, a_length and b_length is given, b comes after a.
 * @param p1 The fork pipes that receive A_h and B_h.
 * @param p2 The fork pipes that receive A_h and B_l.
 * @param p3 The fork pipes that receive A_l and B_h.
 * @param p4 The fork pipes that receive A_l and B_l.
 * @param a_length The number of digits in a.
 * @param b_length The number of digits in b.
 * @param n_half The length, at which a and b should be split, has to be at least the half of the number of digits of max(a, b).
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int pass_integers_down(int *p1, int *p2, int *p3, int *p4, unsigned int a_length, unsigned int b_length, unsigned int n_half) {
    // The total number of digits to be printed.
    unsigned int even_n = 2 * n_half;

    for (unsigned int i = 0; i < even_n; i++) {
        char c = '0';
        // As long as there are digits for a, they should be printed, 0 otherwise.
        if (i < a_length) {
            int status = fgetc(stdin);
            if (status == EOF) return -1;
            c = (char) status;
        }

        // Decide where to write the read digit.
        bool below_half = i < n_half;
        int *p = below_half ? p1 : p3;
        int *q = below_half ? p2 : p4;

        // Write the digit to the pipes.
        if (write(p[FORK_PIPE_DOWN_WRITE], &c, sizeof(char)) == -1) return -1;
        if (write(q[FORK_PIPE_DOWN_WRITE], &c, sizeof(char)) == -1) return -1;
    }
    for (unsigned int i = 0; i < even_n; i++) {
        char c = '0';
        // As long as there are digits for b, they should be printed, 0 otherwise.
        if (i < b_length) {
            int status = fgetc(stdin);
            if (status == EOF) return -1;
            c = (char) status;
        }

        // Decide where to write the read digit.
        bool below_half = i < n_half;
        int *p = below_half ? p1 : p2;
        int *q = below_half ? p3 : p4;

        // Write the digit to the pipes.
        if (write(p[FORK_PIPE_DOWN_WRITE], &c, sizeof(char)) == -1) return -1;
        if (write(q[FORK_PIPE_DOWN_WRITE], &c, sizeof(char)) == -1) return -1;
    }

    return 0;
}

/**
 * @brief Waits for a fork.
 * @details If the wait is not successful, the pipes that belong to it are also closed.
 * @param pid The process id of the fork.
 * @param p The array, where the pipes of the fork are stored. Has to have a length of at least 4.
 * @return 0 if the fork successfully exited, -1 otherwise.
 */
static int wait_for_fork(pid_t pid, int *p) {
    int wstatus;
    if (waitpid(pid, &wstatus, 0) == -1) {
        close_fork_pipes(p);
        return -1;
    }

    if (!WIFEXITED(wstatus) || WEXITSTATUS(wstatus) != EXIT_SUCCESS) {
        close_fork_pipes(p);
        return -1;
    }

    return 0;
}

/**
 * @brief Adds together 4 digits, prints the decimal position of the number and returns the overflow.
 * @param c1 The first digit to be added.
 * @param c2 The second digit to be added.
 * @param c3 The third digit to be added.
 * @param c4 The fourth digit to be added.
 * @param overflow The existing overflow.
 * @return The new overflow for the next digit.
 */
static unsigned long long add_next_digit(char c1, char c2, char c3, char c4, unsigned long long overflow) {
    // Parse all existing digits.
    long d1 = parse_hex_digit(c1);
    long d2 = parse_hex_digit(c2);
    long d3 = parse_hex_digit(c3);
    long d4 = parse_hex_digit(c4);
    long d5 = (long) (overflow % 16);

    long number = d1 + d2 + d3 + d4 + d5;
    long digit = number % 16;
    // Print the decimal position of the number.
    printf("%lx", digit);

    // Calculate the overflow.
    number /= 16;
    overflow /= 16;
    overflow += number;
    return overflow;
}

/**
 * @brief Add together the calculated reversed integers and print them to stdout. The given fork pipes are closed afterwards.
 * @details Each fork received one combination of the fractions of the numbers (fractions created in function pass_integers_down).
 *          These fractions are then added together like this:
 *          A * B = A_h * B_h * 16^n + A_h * B_l * 16^(n/2) + A_l * B_h * 16^(n/2) + A_l * B_l
 * @param f1 The process id of the first fork.
 * @param f2 The process id of the second fork.
 * @param f3 The process id of the third fork.
 * @param f4 The process id of the fourth fork.
 * @param p1 The array of the pipes belonging to the first fork. Has to have a length of at least 4.
 * @param p2 The array of the pipes belonging to the second fork. Has to have a length of at least 4.
 * @param p3 The array of the pipes belonging to the third fork. Has to have a length of at least 4.
 * @param p4 The array of the pipes belonging to the fourth fork. Has to have a length of at least 4.
 * @param n The length of the digits of the input numbers a and b.
 * @param n_half The length where the numbers a and b have been split.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int pass_integers_up(pid_t f1, pid_t f2, pid_t f3, pid_t f4, int *p1, int *p2, int *p3, int *p4, unsigned int n, unsigned int n_half) {
    // Wait for all forks to finish.
    int wait = wait_for_fork(f1, p1) | wait_for_fork(f2, p2)
                | wait_for_fork(f3, p3) | wait_for_fork(f4, p4);
    if (wait == -1) return -1;

    unsigned long long overflow = 0;

    int i = 0;
    while (true) {
        char c1 = '0', c2 = '0', c3 = '0', c4 = '0';
        ssize_t s1 = 1, s2 = 1, s3 = 1, s4 = 1;

        // Read one character from each pipe.
        s1 = read(p1[FORK_PIPE_UP_READ], &c1, sizeof(char));
        if (i >= n_half) {
            s2 = read(p2[FORK_PIPE_UP_READ], &c2, sizeof(char));
            s3 = read(p3[FORK_PIPE_UP_READ], &c3, sizeof(char));
        }
        if (i >= n) s4 = read(p4[FORK_PIPE_UP_READ], &c4, sizeof(char));

        ssize_t s = s1 | s2 | s3 | s4;
        if (s < 0) {
            close_4_sibling_fork_pipes(p1, p2, p3, p4);
            return -1;
        }
        // Return if there are no more digits left to be written.
        if (s == 0 && overflow == 0) {
            if (close_4_sibling_fork_pipes(p1, p2, p3, p4) == -1) return -1;
            return 0;
        }

        // Add together the read digits.
        overflow = add_next_digit(c1, c2, c3, c4, overflow);
        i++;
    }
}

/**
 * @brief If the current process is a fork, then this variable is set to true.
 */
static bool is_fork = false;

/**
 * @brief Perform the integer multiplication by creating forks. (More information in function pass_integers_down and pass_integers_up)
 * @details All integers given by the pipes are on the same line, a before b, and written in reverse, so they can be easily added up.
 * @param p The array of the pipes belonging to the parent of the fork. Has to have a length of at least 4.
 * @param a_length The number of digits in a.
 * @param b_length The number of digits in b.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int fork_multiply_integers(int *p, unsigned int a_length, unsigned int b_length) {
    // The current process is a fork.
    is_fork = true;

    // Redirect the given pipes to stdin and stdout.
    if (redirect_parent_fork_pipes(p) == -1) return -1;

    // If a and b both have a length of 1, then the numbers should be multiplied directly.
    if (a_length == 1 && b_length == 1) {
        int s1 = fgetc(stdin);
        int s2 = fgetc(stdin);
        if (s1 == EOF || s2 == EOF) return -1;

        long a = parse_hex_digit((char) s1);
        long b = parse_hex_digit((char) s2);

        print_hex_reverse(a * b);
        return 0;
    }

    // Set n to the number digits of the bigger number.
    unsigned int n = a_length > b_length ? a_length : b_length;
    unsigned int n_lower = n - (n / 2);

    // The arrays for the fork pipes.
    int p1[4], p2[4], p3[4], p4[4];
    // Open all fork pipes.
    int open = open_fork_pipes(p1) | open_fork_pipes(p2)
                | open_fork_pipes(p3) | open_fork_pipes(p4);

    if (open == -1) {
        close_4_sibling_fork_pipes(p1, p2, p3, p4);
        return -1;
    }

    // Create the first fork on the current process.
    pid_t f1 = fork();
    if (f1 <= 0) {
        close_3_sibling_fork_pipes(p2, p3, p4);
        if (f1 == -1) {
            close_fork_pipes(p1);
            return -1;
        }
        return fork_multiply_integers(p1, n_lower, n_lower);
    }

    // Create the second fork on the current process.
    pid_t f2 = fork();
    if (f2 <= 0) {
        close_3_sibling_fork_pipes(p1, p3, p4);
        if (f2 == -1) {
            close_fork_pipes(p2);
            return -1;
        }
        return fork_multiply_integers(p2, n_lower, n_lower);
    }

    // Create the third fork on the current process.
    pid_t f3 = fork();
    if (f3 <= 0) {
        close_3_sibling_fork_pipes(p1, p2, p4);
        if (f3 == -1) {
            close_fork_pipes(p3);
            return -1;
        }
        return fork_multiply_integers(p3, n_lower, n_lower);
    }

    // Create the fourth fork on the current process.
    pid_t f4 = fork();
    if (f4 <= 0) {
        close_3_sibling_fork_pipes(p1, p2, p3);
        if (f4 == -1) {
            close_fork_pipes(p4);
            return -1;
        }
        return fork_multiply_integers(p4, n_lower, n_lower);
    }

    // Close all unused child fork pipes on the current process.
    int close = close_unused_child_fork_pipes(p1) | close_unused_child_fork_pipes(p2)
                | close_unused_child_fork_pipes(p3) | close_unused_child_fork_pipes(p4);

    if (close == -1) {
        close_4_sibling_fork_pipes(p1, p2, p3, p4);
        return -1;
    }

    // Pass the given integers down to the fork pipes.
    if (pass_integers_down(p1, p2, p3, p4, a_length, b_length, n_lower) == -1) {
        close_4_sibling_fork_pipes(p1, p2, p3, p4);
        return -1;
    }

    // Finally, pass the calculated integers back up to the current process.
    return pass_integers_up(f1, f2, f3, f4, p1, p2, p3, p4, n, n_lower);
}

/**
 * @brief Pipe the required fork pipe to stdin, read the reversed integer from stdin, re-reverse it and print the result to stdout.
 * @param pid The process id of the fork, which is at the top of the recursive calculation.
 * @param p The array of fork pipes. Has to be at least of length 4.
 * @param negative A boolean indicating if the number to be printed is negative.
 * @return 0, if the operation was successful, -1.
 */
static int print_result(pid_t pid, int *p, bool negative) {
    // Set the up read fork pipe to stdin.
    if (dup2(p[FORK_PIPE_UP_READ], STDIN_FILENO) == -1) {
        close_fork_pipes(p);
        return -1;
    }

    // Wait for the top level fork process to finish.
    if (wait_for_fork(pid, p) == -1) return -1;

    unsigned int length;
    // Read the result.
    char *result = read_line(&length);

    if (result == NULL) {
        close_fork_pipes(p);
        return -1;
    }

    if (close_fork_pipes(p) == -1) return -1;

    if (negative) printf("-");

    bool printed = false;
    for (unsigned int i = length; i > 0; i--) {
        char c = result[i - 1];
        // Do not print leading zeros.
        if (printed || c != '0') {
            printed = true;
            printf("%c", c);
        }
    }

    printf("\n");

    free(result);

    return 0;
}

/**
 * @brief Multiplies two hexadecimal integers by creating forks and prints the result to stdout.
 * @param a_line The string, where the first integer is stored.
 * @param b_line The string, where the second integer is stored.
 * @param a_length The number of digits of the first integer.
 * @param b_length The number of digits of the second integer.
 * @return 0 if the operation was successful, -1 otherwise.
 */
static int multiply_integers(char *a_line, char *b_line, unsigned int a_length, unsigned int b_length) {
    char *a = a_line;
    char *b = b_line;

    // Normalise the given integers.
    bool a_negative = normalize_positive_hex_int(&a, &a_length);
    bool b_negative = normalize_positive_hex_int(&b, &b_length);
    bool negative = a_negative != b_negative;

    int p[4];

    // Open pipes for the top level fork.
    if (open_fork_pipes(p) == -1) {
        free(a_line);
        free(b_line);
        return -1;
    }

    int *p_down_write = p + FORK_PIPE_DOWN_WRITE;

    // Write the two integers to the down write fork pipe in reverse.
    int a_write = write_reverse(*p_down_write, a, a_length);
    int b_write = write_reverse(*p_down_write, b, b_length);

    free(a_line);
    free(b_line);

    // Close the now unused for pipe direction.
    if (close_fork_pipe_direction(p_down_write) == -1 || a_write == -1 || b_write == -1) {
        close_fork_pipes(p);
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        close_fork_pipes(p);
        return -1;
    }
    // If the current process is the fork, the integers are multiplied using the fork_multiply_integers function.
    if (pid == 0) return fork_multiply_integers(p, a_length, b_length);
    // If the current process is the main process, all unused fork pipes can be closed.
    if (close_unused_child_fork_pipes(p) == -1) {
        close_fork_pipes(p);
        return -1;
    }
    // Once the multiplication finishes, the result is written to stdout.
    return print_result(pid, p, negative);
}

/**
 * @brief Program entry point.
 * @details Checks arguments and handles errors.
 * @param argc The argument count of argv.
 * @param argv The arguments that the user inputted into the program. For more information please refer to the usage function.
 * @return EXIT_SUCCESS on correct usage and successful memory allocation, EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[]) {
    // Set the program name.
    prog_name = argv[0];

    // If arguments apart from the program name were given, refer to usage.
    if (argc > 1) usage();

    unsigned int a_length, b_length;

    // Allocate dynamic memory for the two integers to be multiplied.
    char *a_line = read_line(&a_length);
    char *b_line = read_line(&b_length);

    if (a_line == NULL || b_line == NULL) {
        if (a_line != NULL) free(a_line);
        if (b_line != NULL) free(b_line);
        fprintf(stderr, "[%s] Error: Could not allocate enough memory to read integers\n", prog_name);
        return EXIT_FAILURE;
    }

    unsigned int a_digit_length, b_digit_length;

    // Check if the two lines are hexadecimal integers.
    if (!is_valid_hex_int(a_line, a_length, &a_digit_length) || !is_valid_hex_int(b_line, b_length, &b_digit_length)) {
        free(a_line);
        free(b_line);
        usage();
    }

    int allowed_length = 1;
    while (a_digit_length > allowed_length) allowed_length <<= 1;

    // Check if a and b have the same number of digits as well as if they are a power of 2.
    if (a_digit_length != b_digit_length || a_digit_length != allowed_length) {
        free(a_line);
        free(b_line);
        usage();
    }

    // A limit for the maximum allowed digit length, as the multiplication for big numbers creates a large number of forks.
    if (a_length > INTMUL_MAX_SAFE_LENGTH) {
        free(a_line);
        free(b_line);
        fprintf(stderr, "[%s] Error: Integers exceed the maximum safe length, which is %i\n", prog_name, INTMUL_MAX_SAFE_LENGTH);
        return EXIT_FAILURE;
    }

    // Perform the integer multiplication and print the output to stdout.
    if (multiply_integers(a_line, b_line, a_length, b_length) == -1) {
        // If the current process is a fork, then no error message should be printed to avoid duplicated error messages.
        if (!is_fork) fprintf(stderr, "[%s] Error: Could not perform integer multiplication\n", prog_name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
