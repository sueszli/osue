#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * @file intmul.c
 * @author Elias GALL - 12019857
 * @brief reads two hexadecimal integer whose length is identical and a power of 2 from stdin and multiplies them
 * @details Multiplies two hex integers by using the formula supplied with the exercise and recursively spawning child processes
 *          until every factor only has one digit.
 * @date 2021-11-17
 */

/**
 * @brief checks if a line entered by the user is valid
 * @details Validates the length of the line and every individual character. The string is expected to end with '\n'
 *          and contain either 1 or an even number of (and only of) the following characters: [A-Fa-f0-9]
 * @param line ... char pointer to the string to be checked
 * @return -1 ... line not valid
 * @return 0 ... valid hex string
 */
static int is_line_valid_hex_string(char *line);

/**
 * @brief reads a line from stdin into the supplied char pointer
 * @details Reads one line from stdin and writes it into the string pointed to by 'line'. Uses 'getline()' to retrieve the line.
 * @param line ... pointer to the string to store the result in
 * @param program_name ... name of the program used in error messages
 * @return size of the string read
 */
static size_t getline_from_stdin(char **line, char *program_name);

/**
 * @brief spawns a child and supplies it with 'line1' and 'line2', as well as 'program_name'
 * @details Forks the current process and if it is a child, the entire program is started again. If the process comes out as parent,
 *          two pipes are created, one to re-route the child's stdin and another to re-route the child's stdout.
 * @param ret ... two value int array, to which the output will be written in the form {PID, stdout_FD}
 * @param line1 ... first argument line passed to child
 * @param line2 ... second argument line passed to child
 * @param program_name ... program name that will be passed to the child
 * @return void
 */
static void spawn_child(int ret[2], char *line1, char *line2, char *program_name);

/**
 * @brief converts single upper case hex char to int
 * @details Converts one single upper case hex char to int.
 * @param x ... upper case hex character
 * @return integer value of the given character
 */
static int hex_char_to_int(char x);

/**
 * @brief adds a string to another
 * @details Calls 'add_to_hex_strings(dest, dest, add)'.
 * @param dest ... destination and first summand, has to be a valid hex string, has to be long enough to store the result
 * @param add ... second summand, has to be a valid hex string and at most as long as dest
 * @return 
 */
static char *add_to_hex_string(char *dest, char *add);

/**
 * @brief adds two strings and writes the result to 'dest'
 * @details Adds 'add1' and 'add2' interpreted as hex-strings character by character (with carry) and writes the result to 'dest'.
 * @param dest ... will contain the sum, has to be large enough for all digits
 * @param add1 ... first summand, has to be a valid hex string according to 'is_line_valid_hex_string()'
 * @param add2 ... second summand, has to be a valid hex string according to 'is_line_valid_hex_string()',
 *                 has to be shorter or equal in length to 'add1'
 * @return dest
 */
static char *add_hex_strings(char *dest, char *add1, char *add2);

/**
 * @brief reads the line returned by the child specified
 * @details Uses 'getline()' to read a line from the output pipe of the child specified.
 * @param child[] ... int array containing child PID at [0] and the file descriptor of the output pipe read end of the child at [1]
 * @return newly allocated string containing the line read
 */
static char *get_line_from_child(int child[]);

/**
 * @brief shifts 'hex' n-characters to the left, filling the created space with '0'
 * @details Shifts the string supplied by n-characters, which is equivalent to a multiplication with 16^n.
 *          Allocates a new line to copy the old one to, and frees the old one.
 * @param hex ... valid hex string
 * @param n ... non-negative int
 * @param program_name ... name of the program used in error messages
 * @return new valid hex string containing the shifted string
 */
static char *shift_to_left(char *hex, int n, char *program_name);

/**
 * @brief lowers all hex letters of 'hex'
 * @details Takes a valid hex string and changes all it's letters to lower case.
 * @param hex ... valid hex string
 * @return void
 */
static void to_lower(char *hex);

/**
 * @brief raises all hex letters of 'hex'
 * @details Takes a valid hex string and changes all it's letters to upper case.
 * @param hex ... valid hex string
 * @return void
 */
static void to_upper(char *hex);

/**
 * @brief main entry point of the program
 * @details Checks that no arguments were supplied. Reads and validates 2 lines from stdin.
 *          If they contain only one digit, multiplies them and returns the result. Otherwise
 *          Creates 4 child processes tasked with computing partial results and sums their results
 *          according to the formula specified in the task description. Results are printed to
 *          stdout.
 * @param argc ... argument counter
 * @param argc ... argument vector
 * @return EXIT_SUCCESS ... success
 * @return EXIT_FAILURE ... failure
 */
int main(int argc, char *argv[]) {
    if (argc > 1) {
        fprintf(stderr, "%s: usage: intmul\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *line1 = NULL;
    getline_from_stdin(&line1, argv[0]);
    char *line2 = NULL;
    getline_from_stdin(&line2, argv[0]);
    int len1 = strlen(line1);
    int len2 = strlen(line2);
    if (len1 != len2) {
        fprintf(stderr, "%s: inputs are not same length\n", argv[0]);
        free(line1);
        free(line2);
        return EXIT_FAILURE;
    }
    if (is_line_valid_hex_string(line1) == 0 && is_line_valid_hex_string(line2) == 0) {
        to_upper(line1);
        to_upper(line2);
        int n = len1 - 1;
        if (len1 == 2) {
            long number1 = strtol(line1, NULL, 16);
            long number2 = strtol(line2, NULL, 16);
            printf("%x\n", (int)(number1 * number2));
            free(line1);
            free(line2);
            return EXIT_SUCCESS;
        } else {
            int split_len = (len1 - 1) / 2;
            char *ah = malloc(sizeof(char) * (split_len + 2));
            if (ah == NULL) {
                fprintf(stderr, "%s: malloc failed - %s\n", argv[0], strerror(errno));
                return EXIT_FAILURE;
            }
            strncpy(ah, line1, split_len);
            ah[split_len] = '\n';
            ah[split_len + 1] = '\0';
            char *al = malloc(sizeof(char) * (split_len + 2));
            if (al == NULL) {
                fprintf(stderr, "%s: malloc failed - %s\n", argv[0], strerror(errno));
                return EXIT_FAILURE;
            }
            strncpy(al, line1 + split_len * sizeof(char), split_len);
            al[split_len] = '\n';
            al[split_len + 1] = '\0';
            char *bh = malloc(sizeof(char) * (split_len + 2));
            if (bh == NULL) {
                fprintf(stderr, "%s: malloc failed - %s\n", argv[0], strerror(errno));
                return EXIT_FAILURE;
            }
            strncpy(bh, line2, split_len);
            bh[split_len] = '\n';
            bh[split_len + 1] = '\0';
            char *bl = malloc(sizeof(char) * (split_len + 2));
            if (bl == NULL) {
                fprintf(stderr, "%s: malloc failed - %s\n", argv[0], strerror(errno));
                return EXIT_FAILURE;
            }
            strncpy(bl, line2 + split_len * sizeof(char), split_len);
            bl[split_len] = '\n';
            bl[split_len + 1] = '\0';

            
            char *result = malloc(sizeof(char) * n * 2 + 2);
            if (result == NULL) {
                fprintf(stderr, "%s: malloc failed - %s\n", argv[0], strerror(errno));
                return EXIT_FAILURE;
            }
            int i = 0;
            for (i = 0; i < n * 2; i++) {
                result[i] = '0';
            }
            result[n * 2] = '\n';
            result[n * 2 + 1] = '\0';
            free(line1);
            line1 = NULL;
            free(line2);
            line2 = NULL;

            int kid_AB_h[2];
            int kid_A_h_B_l[2];
            int kid_A_l_B_h[2];
            int kid_AB_l[2];
            spawn_child(kid_AB_h, ah, bh, argv[0]);
            spawn_child(kid_A_h_B_l, ah, bl, argv[0]);
            spawn_child(kid_A_l_B_h, al, bh, argv[0]);
            spawn_child(kid_AB_l, al, bl, argv[0]);
            free(ah);
            ah = NULL;
            free(al);
            al = NULL;
            free(bh);
            bh = NULL;
            free(bl);
            bl = NULL;
            
            int stat = 0;

            waitpid(kid_AB_h[0], &stat, 0);
            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "%s: child 1 not successful\n", argv[0]);
                wait(&stat);
                wait(&stat);
                wait(&stat);
                return EXIT_FAILURE;
            }
            char *child_line = get_line_from_child(kid_AB_h);
            to_upper(child_line);
            int child_len = strlen(child_line) - 1;
            int offset = n - child_len;
            for (i = offset; i < n; i++) {
                result[i] = child_line[i - offset];
            }
            free(child_line);
            child_line = NULL;

            waitpid(kid_A_l_B_h[0], &stat, 0);
            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "%s: child 2 not successful\n", argv[0]);
                wait(&stat);
                wait(&stat);
                return EXIT_FAILURE;
            }
            child_line = get_line_from_child(kid_A_l_B_h);
            to_upper(child_line);
            child_line = shift_to_left(child_line, n / 2, argv[0]);
            add_to_hex_string(result, child_line);
            free(child_line);
            child_line = NULL;
            
            waitpid(kid_A_h_B_l[0], &stat, 0);
            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "%s: child 3 not successful\n", argv[0]);
                wait(&stat);
                return EXIT_FAILURE;
            }
            child_line = get_line_from_child(kid_A_h_B_l);
            to_upper(child_line);
            child_line = shift_to_left(child_line, n / 2, argv[0]);
            add_to_hex_string(result, child_line);
            free(child_line);
            child_line = NULL;

            waitpid(kid_AB_l[0], &stat, 0);
            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "%s: child 4 not successful\n", argv[0]);
                return EXIT_FAILURE;
            }
            child_line = get_line_from_child(kid_AB_l);
            to_upper(child_line);
            add_to_hex_string(result, child_line);
            free(child_line);
            child_line = NULL;

            to_lower(result);
            printf(result);
            free(result);
            result = NULL;
        }
    } else {
        fprintf(stderr, "%s: invalid inputs\n", argv[0]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static void to_lower(char *hex) {
    int i = 0;
    for (i = 0; i < strlen(hex) - 1; i++) {
        if (hex[i] >= 'A' && hex[i] <= 'F') {
            hex[i] = hex[i] + ('a' - 'A');
        }
    }
}

static void to_upper(char *hex) {
    int i = 0;
    for (i = 0; i < strlen(hex) - 1; i++) {
        
        if (hex[i] >= 'a' && hex[i] <= 'f') {
            hex[i] = hex[i] - ('a' - 'A');
        }
    }
}

static char *shift_to_left(char *hex, int n, char *program_name) {
    int len = strlen(hex);
    char *new_line = malloc(len + 1 + n);
    if (new_line == NULL) {
        fprintf(stderr, "%s: malloc failed - %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int i = 0;
    for (i = 0; i < len - 1; i++) {
        new_line[i] = hex[i];
    }
    free(hex);
    hex = NULL;
    for (; i < len + n - 1; i++) {
        new_line[i] = '0';
    }
    new_line[len + n - 1] = '\n';
    new_line[len + n] = '\0';
    return new_line;
    return hex;
}

static char *get_line_from_child(int child[]) {
    FILE *f = fdopen(child[1], "r");
    char *line = NULL;
    size_t l = 0;
    getline(&line, &l, f);
    fclose(f);
    close(child[1]);
    return line;
}

static void spawn_child(int ret[2], char *line1, char *line2, char *program_name) {
    int kid_input[2];
    if (pipe(kid_input) == -1) {
        fprintf(stderr, "%s: pipe failed - %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int kid_output[2];
    if (pipe(kid_output) == -1) {
        fprintf(stderr, "%s: pipe failed - %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int pid = fork();
    if (pid == -1) {
        fprintf(stderr, "%s: fork failed - %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        fflush(stderr);
        // this process is child
        if (close(kid_input[1]) == -1) {
            fprintf(stderr, "%s: close failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(kid_output[0]) == -1) {
            fprintf(stderr, "%s: close failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (dup2(kid_input[0], STDIN_FILENO) == -1) {
            fprintf(stderr, "%s: dup2 failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(kid_input[0]) == -1) {
            fprintf(stderr, "%s: close failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (dup2(kid_output[1], STDOUT_FILENO) == -1) {
            fprintf(stderr, "%s: dup2 failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(kid_output[1]) == -1) {
            fprintf(stderr, "%s: close failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        execlp(program_name, program_name, NULL);
        fprintf(stderr, "%s: execlp failed - %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // this process is parent
        if (close(kid_input[0]) == -1) {
            fprintf(stderr, "%s: close failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(kid_output[1]) == -1) {
            fprintf(stderr, "%s: close failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        int len = strlen(line1);
        if (write(kid_input[1], line1, len) == -1) {
            fprintf(stderr, "%s: write failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (write(kid_input[1], line2, len) == -1) {
            fprintf(stderr, "%s: write failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(kid_input[1]) == -1) {
            fprintf(stderr, "%s: write failed - %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        ret[0] = pid;
        ret[1] = kid_output[0];
    }
}

static int is_line_valid_hex_string(char *line) {
    int len = strlen(line);
    if ((len % 2 == 0 && len > 2) || len == 1 || line[len - 1] != '\n') {
        return -1;
    }
    int i = 0;
    for (; i < len - 1; i++) {
        if (!((line[i] >= 'a' && line[i] <= 'f') || (line[i] >= 'A' && line[i] <= 'F') || (line[i] >= '0' && line[i] <= '9'))) {
            return -1;
        }
    }
    return 0;
}

size_t getline_from_stdin(char **line, char *program_name) {
    size_t line1_len = 0;
    if (getline(line, &line1_len, stdin) == -1) {
        fprintf(stderr, "%s: getline failed - %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return line1_len;
}

static char *add_to_hex_string(char *dest, char *add) {
    return add_hex_strings(dest, dest, add);
}

static char *add_hex_strings(char *dest, char *add1, char *add2) {
    // last character always newline
    int size1 = strlen(add1) - 2;
    int size2 = strlen(add2) - 2;
    int size_dest = strlen(dest) - 2;
    int i = 0;
    int carry = 0;
    char sum_result[2] = "00";
    // add the two when both exist
    for (i = 0; i <= size1 && i <= size2; i++) {
        int x = hex_char_to_int(add1[size1 - i]) + hex_char_to_int(add2[size2 - i]) + carry;
        sprintf(sum_result, "%02X", x);
        carry = hex_char_to_int(sum_result[0]);
        dest[size_dest - i] = sum_result[1];
    }

    // add the longer one plus carry to dest
    if (size1 == size2 && carry != 0) {
        // only carry left
        dest[size_dest - i] = sum_result[0];
        return dest;
    }
    for (; i <= size1; i++) {
        int x = hex_char_to_int(add1[size1 - i]) + carry;
        sprintf(sum_result, "%02X", x);
        carry = hex_char_to_int(sum_result[0]);
        dest[size_dest - i] = sum_result[1];
    }
    if (carry != 0) {
        dest[size_dest - i] = sum_result[0];
    }
    return dest;
}

static int hex_char_to_int(char x) {
    return x >= 'A' ? x - 'A' + 10 : x - '0';
}