/**
 * @file intmult.c
 * @author Merlin Zalodek, 12019836
 * @date 24.11.2021
 * @brief implementation of task 2 - Integer Multiplication
 * @details The program takes two hexadecimal integers A and B with an equal number of digits as input, 
 *          multiplies them and prints the result. The input is read from stdin and consists of two lines:
 *          the first line is the integer A and the second line is the integer B.
 *          Global variables and definitions are listed below.
 */

/// INCLUSIONS
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>

/// DEFINITIONS
#define CHILD_PROCESSES 4
#define READ_PIPE 0
#define WRITE_PIPE 1
#define PARENT_TO_CHILD_READ 0
#define PARENT_TO_CHILD_WRITE 1
#define CHILD_TO_PARENT_READ 2
#define CHILD_TO_PARENT_WRITE 3

/// TYPE DEFINITIONS
typedef struct input_numbers
{
    char *numberA;
    char *numberB;
    size_t length;
} input_numbers_t;

/// GLOBAL VARIABLES
static const char *program_name;
static bool print_tree = false; ///< not implemented
static input_numbers_t input_numbers; ///< struct for input numbers and length
static int pipe_fds[4 * CHILD_PROCESSES]; ///< File descriptors of read and write end are returned in this specified integer array 'pipe_fds'
static pid_t p_ids[CHILD_PROCESSES]; ///< process ids of child processes

/**
 * @brief This method does read the first two lines from stdin and checks wheter they have the same length and if they are hexadecimal.
 * @details The input numbers and the length of them are saved in the input_numbers struct.
 * @param none
 * @return void 
 */
static void read_input_stdin(void)
{
    input_numbers.numberA = NULL; ///< freed at atexit()
    input_numbers.numberB = NULL;

    size_t length_input_1, length_input_2, buffer_input_1, buffer_input_2;

    getline(&input_numbers.numberA, &buffer_input_1, stdin);
    getline(&input_numbers.numberB, &buffer_input_2, stdin);

    strtok(input_numbers.numberA, "\n"); ///< removes next line-symbol of string
    strtok(input_numbers.numberB, "\n");

    length_input_1 = strlen(input_numbers.numberA);
    length_input_2 = strlen(input_numbers.numberB);

    if (length_input_1 == 0 || length_input_2 == 0)
    {
        fprintf(stderr, "[%s]: No input given in file/stdin.\n", program_name);
        exit(EXIT_FAILURE);
    }

    if (length_input_1 != length_input_2)
    {
        fprintf(stderr, "[%s]: Input (%lu != %lu) does not have the same length.\n", program_name, length_input_1, length_input_2);
        exit(EXIT_FAILURE);
    }

    input_numbers.length = strlen(input_numbers.numberA);

    int problem_counter = 0;
    for (size_t i = 0; i < input_numbers.length; i++)
    {
        if (isxdigit(input_numbers.numberA[i]) == false) ///< The isxdigit() function checks whether a character is a hexadecimal digit character (0-9, a-f, A-F) or not.
        {
            fprintf(stderr, "Digit %c of value %s of line 1 is not hexadecimal.\n", input_numbers.numberA[i], input_numbers.numberA);
            problem_counter++;
        }
        if (isxdigit(input_numbers.numberB[i]) == false)
        {
            fprintf(stderr, "Digit %c of value %s of line 2 is not hexadecimal.\n", input_numbers.numberA[i], input_numbers.numberA);
            problem_counter++;
        }
    }
    if (problem_counter != 0)
    {
        fprintf(stderr, "[%s]: Wrong input - please correct the digits above.\n", program_name);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Method that frees all opened ressources. (Called by atexit() (on error or on normal exit))
 * @details uses global struct input_numbers
 * @param none
 * @return void
 */
static void close_ressources(void)
{
    if (input_numbers.numberA != NULL)
    {
        free(input_numbers.numberA);
    }
    if (input_numbers.numberB != NULL)
    {
        free(input_numbers.numberB);
    }
}

/**
 * @brief Method that converts a hex character into a decimal int
 * @details Using a switch control structure
 * @param hex_number 
 * @return converted int value
 */
static int hex_to_decimal(char c)
{
    switch (c)
    {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A': case 'a': return 10;
        case 'B': case 'b': return 11;
        case 'C': case 'c': return 12;
        case 'D': case 'd': return 13;
        case 'E': case 'e': return 14;
        case 'F': case 'f': return 15;
    default:
        fprintf(stderr, "[%s]: ERROR: cannot convert hex to decimal -> hex_to_decimal -> character not found '%c'\n", program_name, c);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Opens pipes in order to communicate with child processes 
 * @details saved in pipe_fds[16]-array as [1p2cR,1p2cW,1c2pR,1c2pW,2p2cR,2p2cW,2c2pR,2c2pW,3p2cR,3p2cW,3c2pR,3c2pW,4p2cR,4p2cW,4c2pR,4c2pW]
 *                                         [  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15  ]
 *                      1,2,3,4 ... child-process
 *                      p2c ... parent to child
 *                      c2p ... child to parent
 *                      R ... Read
 *                      W ... Write
 * @param none
 * @return void
 */
static void opening_pipes(void)
{
    for (size_t child_process = 0; child_process < CHILD_PROCESSES; child_process++)
    {
        int parent_to_child_pipe[2];
        int child_to_parent_pipe[2];

        if (pipe(parent_to_child_pipe) == -1)
        {
            fprintf(stderr, "[%s]: ERROR: pipe -> parent_to_child_pipe\n", program_name);
            exit(EXIT_FAILURE);
        }
        if (pipe(child_to_parent_pipe) == -1)
        {
            fprintf(stderr, "[%s]: ERROR: pipe -> child_to_parent_pipe\n", program_name);
            exit(EXIT_FAILURE);
        }

        pipe_fds[4 * child_process + PARENT_TO_CHILD_READ] = parent_to_child_pipe[READ_PIPE];
        pipe_fds[4 * child_process + PARENT_TO_CHILD_WRITE] = parent_to_child_pipe[WRITE_PIPE];

        pipe_fds[4 * child_process + CHILD_TO_PARENT_READ] = child_to_parent_pipe[READ_PIPE];
        pipe_fds[4 * child_process + CHILD_TO_PARENT_WRITE] = child_to_parent_pipe[WRITE_PIPE];
    }
}

/**
 * @brief Closing two unused pipe ends [PARENT_TO_CHILD_READ and CHILD_TO_PARENT_WRITE] for four child_processes
 * @details using global array pipe_fds
 * @param none
 * @return void
 */
static void closing_unused_pipe_ends(void)
{
    for (size_t child_process = 0; child_process < CHILD_PROCESSES; child_process++)
    {
        if (close(pipe_fds[4 * child_process + PARENT_TO_CHILD_READ]) == -1)
        {
            fprintf(stderr, "[%s]: ERROR: cannot close pipe -> PARENT_TO_CHILD_READ -> child-process(%zu)\n", program_name, child_process);
            exit(EXIT_FAILURE);
        }
        if (close(pipe_fds[4 * child_process + CHILD_TO_PARENT_WRITE]) == -1)
        {
            fprintf(stderr, "[%s]: ERROR: cannot close pipe -> CHILD_TO_PARENT_WRITE -> child-process(%zu)\n", program_name, child_process);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Closing all pipe ends, which are ...
 * @details saved in global array pipe_fds
 * @param none
 * @return void
 */
static void closing_pipes(void)
{
    for (size_t i = 0; i < 4 * CHILD_PROCESSES; i++)
    {
        if (close(pipe_fds[i]) == -1)
        {
            fprintf(stderr, "[%s]: ERROR: cannot close pipe -> pipe_fds-id%zu\n", program_name, i);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief This method creates 4 child processes via fork(). When pid == 0 we are in the child process: the method
 * duplicates the opened file descriptor (e.g., a pipe’s end) to the closed one
 *      dup2(oldfd, newfd) duplicates oldfd
 *          - New file descriptor uses ID newfd
 *          - (Implicitly) closes the file descriptor newfd (if necessary)
 *          - newfd points to the same open file description like oldfd
 * then exec loads a new program into a process’s memory, executes another program, in the same process (PID remains the same)
 * @details uses global array pipe_fds and saves pid in global array p_ids
 * @param none
 * @return void
 */
static void forking_pipes(void)
{
    for (size_t child_process = 0; child_process < CHILD_PROCESSES; child_process++)
    {
        pid_t pid = fork(); ///< creates a process (copies the process image)
        if (pid == -1)
        {
            fprintf(stderr, "[%s]: ERROR: fork() -> child_process(%zu)\n", program_name, child_process);
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            if (dup2(pipe_fds[4 * child_process + PARENT_TO_CHILD_READ], STDIN_FILENO) == -1)
            {
                fprintf(stderr, "[%s]: ERROR: dup2() -> child_process(%zu) -> PARENT_TO_CHILD_READ\n", program_name, child_process);
                exit(EXIT_FAILURE);
            }
            if (dup2(pipe_fds[4 * child_process + CHILD_TO_PARENT_WRITE], STDOUT_FILENO) == -1)
            {
                fprintf(stderr, "[%s]: ERROR: dup2() -> child_process(%zu)\n", program_name, child_process);
                exit(EXIT_FAILURE);
            }
            closing_pipes();
            if (execlp(program_name, program_name, NULL, NULL) == -1) ///< loads a program -> recursively execute this program in four child processes
            {
                fprintf(stderr, "[%s]: ERROR: execlp() -> child_process(%zu)\n", program_name, child_process);
                exit(EXIT_FAILURE);
            }
        }
        p_ids[child_process] = pid;
    }
}

/**
 * @brief This method splits the input numbers into 4 parts -> Ah, Al, Bh, Bl. Each part consists of n/2 digits. The splitted numbers are written to the child processes via ...
 * @details uses global struct childrens_input for accessing input numbers & length and uses global array pipe_fds
 * @param none
 * @return void
 */
static void writing_splitted_parts_to_children(void)
{
    int newLength = input_numbers.length / 2;
    char A_h[newLength];
    char A_l[newLength];
    char B_h[newLength];
    char B_l[newLength];

    for (size_t i = 0; i < newLength; i++)
    {
        A_h[i] = input_numbers.numberA[i];
        B_h[i] = input_numbers.numberB[i];
        A_l[i] = input_numbers.numberA[newLength + i];
        B_l[i] = input_numbers.numberB[newLength + i];
    }

    FILE *childrens_input[CHILD_PROCESSES];
    for (size_t child_process = 0; child_process < CHILD_PROCESSES; child_process++)
    {
        childrens_input[child_process] = fdopen(pipe_fds[4 * child_process + PARENT_TO_CHILD_WRITE], "w");
    }

    fprintf(childrens_input[0], "%.*s\n%.*s\n", newLength, A_h, newLength, B_h); ///< %.*s -> The precision is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted. -> https://www.cplusplus.com/reference/cstdio/printf/
    fprintf(childrens_input[1], "%.*s\n%.*s\n", newLength, A_h, newLength, B_l);
    fprintf(childrens_input[2], "%.*s\n%.*s\n", newLength, A_l, newLength, B_h);
    fprintf(childrens_input[3], "%.*s\n%.*s\n", newLength, A_l, newLength, B_l);

    for (size_t child_process = 0; child_process < CHILD_PROCESSES; child_process++)
    {
        if (fclose(childrens_input[child_process]) == -1)
        {
            fprintf(stderr, "[%s]: ERROR: cannot close file (fclose) -> child-process(%zu) -> writing_splitted_parts_to_children()\n", program_name, child_process);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief This method reads the output of the child processes and put them together by calculating in a way that size does not matter for C data types.
 * Result is returned in stdout.
 * @details uses global struct childrens_input for accessing input numbers & length and uses global array pipe_fds
 * @param none
 * @return void
 */
static void reading_calculating_return(void)
{
    FILE *childrens_output[CHILD_PROCESSES];

    for (size_t child_process = 0; child_process < CHILD_PROCESSES; child_process++)
    {
        childrens_output[child_process] = fdopen(pipe_fds[4 * child_process + CHILD_TO_PARENT_READ], "r");
    }

    char *A_h_B_h = NULL;
    char *A_h_B_l = NULL;
    char *A_l_B_h = NULL;
    char *A_l_B_l = NULL;
    ssize_t A_h_B_h_length, A_h_B_l_length, A_l_B_h_length, A_l_B_l_length;
    size_t A_h_B_h_buffer, A_h_B_l_buffer, A_l_B_h_buffer, A_l_B_l_buffer;

    if (((A_h_B_h_length = getline(&A_h_B_h, &A_h_B_h_buffer, childrens_output[0])) == -1) ||
        ((A_h_B_l_length = getline(&A_h_B_l, &A_h_B_l_buffer, childrens_output[1])) == -1) ||
        ((A_l_B_h_length = getline(&A_l_B_h, &A_l_B_h_buffer, childrens_output[2])) == -1) ||
        ((A_l_B_l_length = getline(&A_l_B_l, &A_l_B_l_buffer, childrens_output[3])) == -1))
    {
        fprintf(stderr, "[%s]: ERROR: getline() failed -> while reading a child result an error occured\n", program_name);
        exit(EXIT_FAILURE);
    }

    for (size_t child_process = 0; child_process < CHILD_PROCESSES; child_process++)
    {
        if (fclose(childrens_output[child_process]) == -1)
        {
            fprintf(stderr, "[%s]: ERROR: cannot close file (fclose) -> child-process(%zu) -> reading_calculating_return()\n", program_name, child_process);
            exit(EXIT_FAILURE);
        }
    }

    strtok(A_h_B_h, "\n"); ///< removes next line-symbol of string
    strtok(A_h_B_l, "\n");
    strtok(A_l_B_h, "\n");
    strtok(A_l_B_l, "\n");
    A_h_B_h_length = strlen(A_h_B_h) - 1;
    A_h_B_l_length = strlen(A_h_B_l) - 1;
    A_l_B_h_length = strlen(A_l_B_h) - 1;
    A_l_B_l_length = strlen(A_l_B_l) - 1;

    char result[2 * input_numbers.length + 1];
    memset(result, 0, 2 * input_numbers.length);
    result[2 * input_numbers.length] = '\0';

    /// a way to add the four intermediate results together one digit at a time while keeping track of the carry
    int carry = 0;
    for (int i = 2 * input_numbers.length - 1; i >= 0; i--)
    {
        if (i >= input_numbers.length + input_numbers.length / 2) ///< Al * Bl
        {
            result[i] = A_l_B_l_length <= 0 ? 0 : A_l_B_l[A_l_B_l_length--];
        }
        else if (i >= input_numbers.length) ///< Ah * Bl * 16^n/2 + Al * Bh * 16^n/2 + Al * Bl 
        {
            int tmp1 = A_l_B_l_length < 0 ? 0 : hex_to_decimal(A_l_B_l[A_l_B_l_length--]);
            int tmp2 = A_l_B_h_length < 0 ? 0 : hex_to_decimal(A_l_B_h[A_l_B_h_length--]);
            int tmp3 = A_h_B_l_length < 0 ? 0 : hex_to_decimal(A_h_B_l[A_h_B_l_length--]);

            int sum = tmp1 + tmp2 + tmp3 + carry;
            carry = sum / 16;

            char decimal_to_hex[2];
            sprintf(decimal_to_hex, "%x", (sum % 16));
            result[i] = decimal_to_hex[0];
        }
        else ///< Ah * Bh * 16^n + Ah * Bl * 16^n/2 + Al * Bh * 16^n/2 + Al * Bl
        {
            int tmp1 = A_l_B_l_length < 0 ? 0 : hex_to_decimal(A_l_B_l[A_l_B_l_length--]);
            int tmp2 = A_l_B_h_length < 0 ? 0 : hex_to_decimal(A_l_B_h[A_l_B_h_length--]);
            int tmp3 = A_h_B_l_length < 0 ? 0 : hex_to_decimal(A_h_B_l[A_h_B_l_length--]);
            int tmp4 = A_h_B_h_length < 0 ? 0 : hex_to_decimal(A_h_B_h[A_h_B_h_length--]);

            int sum = tmp1 + tmp2 + tmp3 + tmp4 + carry;
            carry = sum / 16;

            char decimal_to_hex[2];
            sprintf(decimal_to_hex, "%x", (sum % 16));
            result[i] = decimal_to_hex[0];
        }
    }
    free(A_h_B_h);
    free(A_h_B_l);
    free(A_l_B_h);
    free(A_l_B_l);
    fprintf(stdout, "%s\n", result);
}

/**
 * @brief This method causes the parent process to wait for its children to terminate.
 * @details uses global array p_ids to access children process ids
 * @param none
 * @return void
 */
static void waiting_for_child_processes(void)
{
    for (size_t child_process = 0; child_process < CHILD_PROCESSES; child_process++)
    {
        int status;
        if (waitpid(p_ids[child_process], &status, 0) == -1) ///< waitpid() -> Wait on a specific child process
        {
            fprintf(stderr, "[%s]: ERROR: waitpid() -> child-process(%zu) -> waiting_for_child_processes()\n", program_name, child_process);
            exit(EXIT_FAILURE);
        }

        if (WEXITSTATUS(status) != EXIT_SUCCESS)
        {
            fprintf(stderr, "[%s]: ERROR: An error occurred while the child process(%zu) was working -> waiting_for_child_processes()\n", program_name, child_process);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Main Method of Integer Multiplication
 * @details see program description above
 * @param argc 
 * @param argv 
 * @return int - if successfull 0
 */
int main(int argc, char **argv)
{
    program_name = argv[0];
    if (atexit(close_ressources) < 0)
    {
        fprintf(stderr, "[%s]: Error: Cannot register a function to be called on exit.\n", program_name);
        exit(EXIT_FAILURE);
    }

    int option_counter = 0;
    int c;
    while ((c = getopt(argc, argv, "t")) != -1)
    {
        switch (c)
        {
        case 't':
            print_tree = true;
            fprintf(stderr, "Printing tree not implemented!\n");
            option_counter++;
            break;
        case '?':
            if (isprint(optopt)) ///< checks whether a character is a printable character or not.
            {
                fprintf(stderr, "[%s]: Unknown option `-%c'.\n", program_name, optopt);
                exit(EXIT_FAILURE);
            }
            else
            {
                fprintf(stderr, "[%s]: Unknown option character '%x'.\n", program_name, optopt);
                exit(EXIT_FAILURE);
            }
        default:
            fprintf(stderr, "[%s]: Error found in 'getopt'.\n", program_name);
            exit(EXIT_FAILURE);
        }
    }

    if ((argc > 2) || (option_counter > 1))
    {
        fprintf(stderr, "[%s]: Wrong user input.\n", program_name);
        exit(EXIT_FAILURE);
    }

    read_input_stdin();

    if (input_numbers.length == 1)
    {
        int result = hex_to_decimal(input_numbers.numberA[0]) * hex_to_decimal(input_numbers.numberB[0]);
        fprintf(stdout, "%x", result); ///< %x is format specifier for hex lower case
        exit(EXIT_SUCCESS);
    }

    if ((input_numbers.length % 2) != 0)
    {
        fprintf(stderr, "[%s]: Number of digits is not even.\n", program_name);
        exit(EXIT_FAILURE);
    }

    opening_pipes();
    forking_pipes();
    closing_unused_pipe_ends();
    writing_splitted_parts_to_children();
    reading_calculating_return();
    waiting_for_child_processes();
    exit(EXIT_SUCCESS);
}
