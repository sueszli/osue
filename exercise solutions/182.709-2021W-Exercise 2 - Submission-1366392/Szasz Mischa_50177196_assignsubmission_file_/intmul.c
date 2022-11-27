/**
 * @file main.c
 * @author Mischa Szasz <e12025957>
 * @date 10.12.2021
 *
 * @brief Intmul main program module.
 * 
 * This program multiplies two hexadecimal numbers with each other recursively using forks and pipes to communicate.
 * The program will only finish with a hexadecimal number as it's result if both inputs are hexadecimal numbers with a length representing a power of two.
 **/

#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <wait.h>
#include <ctype.h>

#define PIPE_PARENT_READ readpipe[0] // Pipe from which the parent process reads its child processes results.
#define PIPE_CHILD_WRITE readpipe[1] // Pipe from which the child processes write data to the parent process.
#define PIPE_CHILD_READ writepipe[0] // Pipe from which the child processes read the input data from the parent process.
#define PIPE_PARENT_WRITE writepipe[1] // Pipe fromj which the parent process writes the input data to its child processes.

// method declaration
// all doxygen documentation at the implementation of the methods
void print_error_and_exit(char *input, char *program_name);
void wait_for(pid_t child, char* program_name);
void add_number(char *src, int *dest, int dest_offset);
void recalculate_number(int *array, int size);
pid_t exec_command(char *incmd, char *line1, int l1, char *line2, int l2, int *out_PARENT_READ, char* program_name);
void copy_reverse(char *src, char *dst);
char *get_input_line(int *out_length);
int prepare_number(char *line, int line_length, char *out_number);
int check_length(const int l1, const int l2);
int char_to_number(const char c);
int check_error(int return_code, char* message, char* program_name);

/**
 * Program entry point.
 * @brief The program starts here. This function implements the main programm logic used to run it.
 * @param argc The argument counter. Must always be 1 when calling this program.
 * @param argv The argument vector. No commandline arguments may be specified when running this program.
 * @return Returns EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char **argv)
{
    // if arguments are specified the call is incorrect
    if (argc > 1) {
        print_error_and_exit("No input arguments may be specified.", argv[0]);
        exit(EXIT_FAILURE);
    }

    // input handling
    int line1_length;
    char *line1 = get_input_line(&line1_length);
    if (line1 == NULL)
    {
        print_error_and_exit("Could not read first line.", argv[0]);
    }

    int line2_length;
    char *line2 = get_input_line(&line2_length);
    if (line2 == NULL)
    {
        free(line1);
        print_error_and_exit("Could not read second line.", argv[0]);
    }

    char number1[line1_length + 1];
    char number2[line2_length + 1];

    int invalid_input = check_length(line1_length, line2_length);
    if (invalid_input){
        print_error_and_exit("One of your numbers has odd digits, they have different length or no number was provided.", argv[0]);
    }

    invalid_input = invalid_input || prepare_number(line1, line1_length, number1);
    invalid_input = invalid_input || prepare_number(line2, line2_length, number2);

    free(line1);
    free(line2);

    if (invalid_input)
    {
        print_error_and_exit("One of your inputs contains a non-hexadecimal character.", argv[0]);
    }

    // processing
    const int number_length = line1_length;

    // calculate result, if the length of both numbers is 1
    if (number_length == 1)
    {
        const int a = check_error(char_to_number(number1[0]), "Input is not a number.", argv[0]);
        const int b = check_error(char_to_number(number2[0]), "Input is not a number.", argv[0]);
        fprintf(stdout, "%x\n", a * b);
        return EXIT_SUCCESS;
    }

    // ignore closing pipes
    signal(SIGPIPE, SIG_IGN);

    // program name
    char *command = argv[0];

    // pointer arithmetik!
    // number1, number2 pointer of originalzahlen (länge n)
    // a_low, etc. pointer auf aufgeteilte Zahlen mit länge n/2
    const int half_number = number_length / 2;
    char *a_low = number1 + half_number;
    char *a_high = number1;
    char *b_low = number2 + half_number;
    char *b_high = number2;

    // fork & exec four childprocesses

    int parentread_pipe1;
    // inputs: name des programms, pointer, länge, pointer, länge, adresse der pipe
    pid_t child1 = exec_command(command, a_high, half_number, b_high, half_number, &parentread_pipe1, argv[0]);

    int parentread_pipe2;
    pid_t child2 = exec_command(command, a_high, half_number, b_low, half_number, &parentread_pipe2, argv[0]);

    int parentread_pipe3;
    pid_t child3 = exec_command(command, a_low, half_number, b_high, half_number, &parentread_pipe3, argv[0]);

    int parentread_pipe4;
    pid_t child4 = exec_command(command, a_low, half_number, b_low, half_number, &parentread_pipe4, argv[0]);

    // maxlänge der zahl
    const int buflength = number_length * 2;

    // vier kombinationen
    char ahbh[buflength + 1];
    char ahbl[buflength + 1];
    char albh[buflength + 1];
    char albl[buflength + 1];

    int result[buflength];
    // setzt resultbuffer auf null
    memset(result, 0, buflength * sizeof(int));

    // collect results
    wait_for(child1, argv[0]);
    // read answer from pipe into char[]
    const ssize_t r1 = check_error(read(parentread_pipe1, ahbh, buflength), "Error on Pipe-Read.", argv[0]);
    ahbh[r1] = 0;

    wait_for(child2, argv[0]);
    const ssize_t r2 = check_error(read(parentread_pipe2, ahbl, buflength), "Error on Pipe-Read.", argv[0]);
    ahbl[r2] = 0;

    wait_for(child3, argv[0]);
    const ssize_t r3 = check_error(read(parentread_pipe3, albh, buflength), "Error on Pipe-Read.", argv[0]);
    albh[r3] = 0;

    wait_for(child4, argv[0]);
    const ssize_t r4 = check_error(read(parentread_pipe4, albl, buflength), "Error on Pipe-Read.", argv[0]);
    albl[r4] = 0;

    // process results
    char reverse_ahbh[buflength + 1];
    char reverse_ahbl[buflength + 1];
    char reverse_albh[buflength + 1];
    char reverse_albl[buflength + 1];

    copy_reverse(ahbh, reverse_ahbh);
    copy_reverse(ahbl, reverse_ahbl);
    copy_reverse(albl, reverse_albl);
    copy_reverse(albh, reverse_albh);

    // verkehrt im array im endeffekt
    add_number(reverse_albl, result, 0);
    add_number(reverse_albh, result, half_number);
    add_number(reverse_ahbl, result, half_number);
    add_number(reverse_ahbh, result, number_length);

    // handle overflow
    recalculate_number(result, buflength);

    // print result
    for (int j = buflength - 1; j >= 0; --j)
    {
        fprintf(stdout, "%x", result[j]);
    }
    fprintf(stdout, "\n");

    return EXIT_SUCCESS;
}

/**
 * Prints an error to stderr and exits the programm with EXIT_FAILURE.
 * @param input The error message to be printed.
 * @param program_name The program name to specify which program is writing the error. Should typically be argv[0]
 */
void print_error_and_exit(char* input, char* program_name){
    fprintf(stderr, "[%s] %s\n", program_name, input);
    exit(EXIT_FAILURE);
}

/**
 * Checks if the input character is a valid hexadecimal character.
 * @brief Returns if input character is hexadecimal.
 * @param c The character to be checked.
 * @return 1 if the character is hexadecimal, 0 otherwise.
 */
int is_hex_char(const char c)
{
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int char_to_number(const char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    return -1;
}

/**
 * Copies the input string in reverse to another string.
 * @param src The input string to be reversed.
 * @param dst The output string to which the reversed input is written.
 */
void copy_reverse(char *src, char *dst)
{
    int i = strlen(src) - 2;
    int j = 0;
    while (i >= 0)
    {
        dst[j++] = src[i--];
    }
    dst[j] = 0;
}

/**
 * Wait for the process with the process-id child to finish. Exits with failure if the child-process
 * has exited with failure.
 * @brief Wait for the child process to finish.
 * @param child The process for which the method should wait.
 * @param program_name The program name, typically argv[0]. Used in case of an error in the exit message.
 */
void wait_for(pid_t child, char *program_name)
{
    int status;
    // parameter: prozess, &status ist adressausgabe, 0 ist "waiting-code"
    pid_t w = waitpid(child, &status, 0);
    if (w == -1)
    {
        print_error_and_exit("Error waiting for child process.", program_name);
    }
    else
    {
        int exit_status = WEXITSTATUS(status);
        if (exit_status == EXIT_FAILURE)
        {
            char message[100];
            sprintf(message, "Child-Process [%i] terminated with failure.", w);
            print_error_and_exit(message, program_name);
        }
    }
}

/**
 * Add a hexadecimal to another hexadecimal numbers with an offset that specifies how many magnitudes of power are between the two numbers.
 * If dest = 0, src = 1 and offset = 4, then the calculation will result in 0 + 1 * 2^4. If the offset is 0 both numbers are of equal magnitude.
 * @brief Add two hexadecimal numbers with an offset.
 * @param src First hexadecimal number.
 * @param dest Second hexadecimal number to which the first number (src) should be added.
 * @param dest_offset Offset of power (how many magnitudes of power the src number is greater than dest). 
 */
void add_number(char *src, int *dest, int dest_offset)
{
    int i = 0;
    while (src[i] != 0)
    {
        dest[dest_offset++] += char_to_number(src[i++]);
    }
}

/**
 * Recalculates the hexadecimal number by checking for overflow
 * @param array The hexadecimal number array after adding all calculations to each other.
 * @param size Maximum length of the output number in hexadecimal digits.
 */
void recalculate_number(int *array, int size)
{
    int tmp = 0;
    for (int w = 0; w < size; ++w)
    {
        tmp = array[w] >> 4; // dividiert durch 16
        array[w] = array[w] % 16; // rest wird reingeschrieben
        array[w + 1] = array[w + 1] + tmp; // wird zur nächsten Stelle
    }
}

/**
 * Splits process into parent and child process. Execution of both processes continues after the fork. Creates all the pipes
 * necessary to communicate between parent and child processes. 
 * @brief Splits process into parent and child and executes the same program on both.
 * @param incmd Name of the process to run on the child.
 * @param line1 Input line one.
 * @param l1 Length of input line1.
 * @param line2 Input line two.
 * @param l2 Length of input line2.
 * @param out_PARENT_READ Adress of the pipe to which the child-processes must write their result.
 * @param program_name The program name, typically argv[0]. Used in case of an error in the exit message.
 * @return Returns the child process id.
 */
pid_t exec_command(char *incmd, char *line1, int l1, char *line2, int l2, int *out_PARENT_READ, char* program_name)
{
    int writepipe[2] = {-1, -1}, /* parent -> child */
        readpipe[2] = {-1, -1};  /* child -> parent */
    pid_t childpid;

    if (pipe(readpipe) < 0)
    {
        perror("could not create read pipe");
        exit(EXIT_FAILURE);
    }

    if (pipe(writepipe) < 0)
    {
        perror("could not create write pipe");
        exit(EXIT_FAILURE);
    }

    if ((childpid = fork()) < 0)
    {
        perror("could not fork");
        exit(EXIT_FAILURE);
    }
    else if (childpid == 0)
    {
        /* in the child */
        check_error(close(PIPE_PARENT_WRITE), "Error on Pipe-Close.", program_name);
        check_error(close(PIPE_PARENT_READ), "Error on Pipe-Close.", program_name);

        check_error(dup2(PIPE_CHILD_READ, STDIN_FILENO), "Error on fd-duplication.", program_name);
        check_error(close(PIPE_CHILD_READ), "Error on Pipe-Close.", program_name);

        check_error(dup2(PIPE_CHILD_WRITE, STDOUT_FILENO), "Error on fd-duplication.", program_name);

        check_error(close(PIPE_CHILD_WRITE), "Error on Pipe-Close.", program_name);
        check_error(execlp(incmd, incmd, (char *)NULL), "Error on execution.", program_name);
        exit(EXIT_FAILURE); // this never happens
    }
    else
    {
        /* in the parent */
        check_error(write(PIPE_PARENT_WRITE, line1, l1), "Error on Pipe-Write.", program_name);
        check_error(write(PIPE_PARENT_WRITE, "\n", 1), "Error on Pipe-Write.", program_name);
        check_error(write(PIPE_PARENT_WRITE, line2, l2), "Error on Pipe-Write.", program_name);
        check_error(write(PIPE_PARENT_WRITE, "\n", 1), "Error on Pipe-Write.", program_name);

        check_error(close(PIPE_CHILD_READ), "Error on Pipe-Close.", program_name);
        check_error(close(PIPE_CHILD_WRITE), "Error on Pipe-Close.", program_name);

        *out_PARENT_READ = PIPE_PARENT_READ;
        return childpid;
    }
}

/**
 * Reads one input line from the input-stream stdin.
 * @param out_length Pointer to which the length of the input is written.
 * @return char* at which the read input line is written to. This is automatically malloc'ed by getline(3) and must be freed before program termination.
 */
char* get_input_line(int* out_length)
{
    ssize_t read = 0;
    char *line = NULL;
    size_t size_read = 0;

    read = getline(&line, &size_read, stdin);
    if (read == -1)
    {
        return NULL;
    }
    *out_length = read - 1; // -1 because of the \n char at the end
    return line;
}

/**
 * Check if a string only consists of hexadecimal characters and copies them to another string.
 * @param line Input line which is to be checked for hexadecimal characters.
 * @param line_length Length of input line.
 * @param out_number String to which the copied number is written. This is done to be able to free() the line as it is malloc'ed before being input to the method.
 * @return Returns 1 of the input contains an invalid character, 0 if whole input is valid.
 */
int prepare_number(char *line, int line_length, char *out_number)
{
    for (int i = 0; i < line_length; ++i)
    {
        char c = *line++;
        if (!is_hex_char(c))
        {
            return 1;
        }
        *out_number++ = c;
    }
    *out_number = 0;
    return 0;
}

/**
 * Checks if the lengths of both inputs are valid. The input lengths must be both of the same length, have an even length (which is not length 0).
 * @param l1 First length.
 * @param l2 Second length.
 * @param out_number String to which the copied number is written. This is done to be able to free() the line as it is malloc'ed before being input to the method.
 * @return Returns 1 if the lengths are invalid, 0 if the lengths are valid.
 */
int check_length(const int l1, const int l2)
{
    if ((l1 != l2) 
    || (l1 == 0)
    || (l1 > 1 && (l1 % 2 == 1)))
    {
        return 1;
    }
    return 0;
}

/**
 * Checks if the return_code is not -1 which would indicate an error in another method.
 * @param return_code The return code to be checked.
 * @param message If the return code is invalid this message will be printed to stderr.
 * @param program_name The program name, typically argv[0]. Used in case of an error in the exit message.
 * @return Returns the exit code (unless it exits the program beforehand).
 */
int check_error(int return_code, char* message, char* program_name)
{
    if (return_code == -1)
    {
        print_error_and_exit(message,  program_name);
    }
    return return_code;
}
