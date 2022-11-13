/**
 * @file intmul.c
 * @author Lucas Wolkersdorfer/11922587
 * @date 11.12.2021
 * @brief Assignement 2 intmul solution - implementation
 * 
 * @details program takes to hexadecimal numbers as an input and adds them
 */

#include "intmul.h"

/** 
 * @brief stores the name of the program
 */
static const char *PROGRAM_NAME; 

/** 
 * @brief int array with all out pipes.
 */
static int pipe_out[4][2];

/** 
 * @brief processor id array for the 4 children that are forked
 */
static pid_t children[4];

/**
 * @brief adds power amount of zeros to the char array,
 * we do this to replicate the *16^power, since *16 in decimal on a hex number is like adding a 0
 * 
 * @param number the pointer to the number/char array that gets the zeros added
 * @param power the amout of zeros added
 */
static void add_zeros(char *number, size_t power)
{

    int length = strlen(number);
    int new_length = length + power; // our new number has power amount of zeros added to it
    for (size_t i = length; i < new_length; i++) // we start at the end of the number and add power zeros
    {
        number[i] = '0';
    }
    number[new_length] = '\0';
}

/**
 * @brief gets the 4 results from the 4 children
 * and safes them in the array results
 * 
 * @param results the array in which the 4 numbers are stored
 */
void get_results_children(char **results)
{

    for (int i = 0; i < 4; i++)
    {

        FILE *pipe_fd = fdopen(pipe_out[i][0], "r");

        if (pipe_fd == NULL)
        {
            fprintf(stderr, "[%s] ERROR: Couldnt open pipe for read operation : %s \n", PROGRAM_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }
        results[i] = NULL;
        size_t lencap = 0;
        ssize_t len = 0;
        if ((len = getline(&results[i], &lencap, pipe_fd)) == -1)
        {
            for (int j = 0; j < i + 1; j++) // if there is an error with getline we free all previously allocated memory
            {
                fclose(pipe_fd); // and also all opened file descriptors
                free(results[j]);
                close(pipe_out[j][0]);
            }
            fprintf(stderr, "[%s] ERROR: Couldnt get line from pipe : %s \n", PROGRAM_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }
        fclose(pipe_fd);
        close(pipe_out[i][0]);
        results[i][len - 1] = '\0';
    }
}

/**
 * @brief waits for the childrens completion and exits with EXIT_FAILURE if a child exits with EXIT_FAILURE
 * 
 */
void wait_children()
{
    int status[4];
    for (int i = 0; i < 4; i++)
    {
        if (waitpid(children[i], &status[i], 0) < 0)
        {
            fprintf(stderr, "[%s] ERROR: While waiting for child process : %s \n", PROGRAM_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (WEXITSTATUS(status[i]) == 1)
        {
            fprintf(stderr, "[%s] ERROR: Child encountered Error \n", PROGRAM_NAME);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief splits the two inputs that are stored in numbers array into 4 strings of equal length
 * then opens in pipes, forks, and the parent sends the permutations of the 4 strings to the 4 children.
 * the childs sets the pipes to the right stream and the program is executed again, recursively.
 * 
 * @param length length that the 4 halfed strings will be
 * @param numbers array that stors the 2 numbers/char arrays
 */
void fork_children(size_t length, char **numbers)
{
    char half_strings[4][length + 2]; // all 4 half strings are of the same length, +2 for new line and terminating null byte

    for (int i = 0; i < length; i++) // correctly splits up the 2 char arrays into 4 of half the size
    {
        half_strings[0][i] = numbers[0][i]; //Ah
        half_strings[1][i] = numbers[1][i]; //Bh
        half_strings[2][i] = numbers[0][length + i]; //Al
        half_strings[3][i] = numbers[1][length + i]; //Bl
    }

    free(numbers[0]); // we dont need the numbers array anymore so we free it here
    free(numbers[1]);

    for (int i = 0; i < 4; i++)
    {
        half_strings[i][length] = '\n';
        half_strings[i][length + 1] = '\0';
    }

    int pipe_in[4][2]; //one in pipe for all children, out pipes int array is global
    for (int i = 0; i < 4; i++)
    {
        if (pipe(pipe_in[i]) == -1)
        {
            fprintf(stderr, "[%s] ERROR: Couldnt open pipe : %s \n", PROGRAM_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (pipe(pipe_out[i]) == -1)
        {
            fprintf(stderr, "[%s] ERROR: Couldnt open pipe : %s \n", PROGRAM_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }
        children[i] = fork(); // children array is global because we have to access them later at waitpid
        switch (children[i])
        {
        case -1: // gets into this case if fork fails
            fprintf(stderr, "[%s] ERROR: Cannot fork! : %s \n", PROGRAM_NAME, strerror(errno));
            break;

        case 0: // children get into this case, closes unneccessary pipes, redirects pipes accordingly and calls execlp with our program name to start recursion

            close(pipe_in[i][1]);
            close(pipe_out[i][0]);

            if ((dup2(pipe_in[i][0], STDIN_FILENO)) < 0)
            {
                fprintf(stderr, "[%s] ERROR: Couldnt duplicate file descriptor! : %s \n", PROGRAM_NAME, strerror(errno));
                exit(EXIT_FAILURE);
            }

            close(pipe_in[i][0]);

            if ((dup2(pipe_out[i][1], STDOUT_FILENO)) < 0)
            {
                fprintf(stderr, "[%s] ERROR: Couldnt duplicate file descriptor! : %s \n", PROGRAM_NAME, strerror(errno));
                exit(EXIT_FAILURE);
            }

            close(pipe_out[i][1]);

            if (execlp(PROGRAM_NAME, PROGRAM_NAME, NULL) < 0)
            {
                fprintf(stderr, "[%s] ERROR: Couldnt replace process image! : %s \n", PROGRAM_NAME, strerror(errno));
                exit(EXIT_FAILURE);
            }
            break;

        default: // parent gets into this case, closes unneccessary pipes and writes to all 4 children

            close(pipe_in[i][0]);
            close(pipe_out[i][1]);

            FILE *pipe_fd = fdopen(pipe_in[i][1], "w");

            if (pipe_fd == NULL)
            {
                fprintf(stderr, "[%s] ERROR: Couldnt open pipe for write operation! : %s \n", PROGRAM_NAME, strerror(errno));
            }

            if (i == 0)
            {
                fprintf(pipe_fd, "%s", half_strings[0]);
                fprintf(pipe_fd, "%s", half_strings[1]);
            }
            else if (i == 1)
            {
                fprintf(pipe_fd, "%s", half_strings[0]);
                fprintf(pipe_fd, "%s", half_strings[3]);
            }
            else if (i == 2)
            {
                fprintf(pipe_fd, "%s", half_strings[1]);
                fprintf(pipe_fd, "%s", half_strings[2]);
            }
            else
            {
                fprintf(pipe_fd, "%s", half_strings[2]);
                fprintf(pipe_fd, "%s", half_strings[3]);
            }
            fclose(pipe_fd); // we dont have to flush because fclose automatically does that
            close(pipe_in[i][1]);
            break;
        }
    }
}

/**
 * @brief checks if an input of a hexadecimal number was correct. incorrect means: not divisible by 2(unless its 1), emtpy, it includes a non hexadecimal digit 
 * 
 * @param input the string that is checked
 * @return int returns 1 if input is correct, -1 otherwise
 */
int check_input(char *input)
{
    size_t length = strlen(input) - 1;

    if (length != 1)
    {
        if (length % 2 != 0)
        {
            fprintf(stderr, "[%s] ERROR: Hexadecimal number was not divideable by two! \n", PROGRAM_NAME);
            return -1;
        }
    }

    if (length < 1)
    {
        fprintf(stderr, "[%s] ERROR: No Hexadecimal number entered! \n", PROGRAM_NAME);
        return -1;
    }

    for (int i = 0; i < length; i++)
    {
        if (!isxdigit(input[i]))
        {
            fprintf(stderr, "[%s] ERROR: Found a character that is not a hexadecimal digit! \n", PROGRAM_NAME);

            return -1;
        }
    }

    return 1;
}

/**
 * @brief adds to hexadecimal numbers together, stores it in the first number
 *  
 * @details does this by getting both digits in the char array,
 * converting them to int and them adding them. If there is a carry over from the last addition, add that too,
 * and if result is larger than 16 add a carry over. its important to shift where we access the second number by offset,
 * since the first digit of both numbers could be at different indices if the second number is smaller.
 * 
 * @param number_1 number that gets the result of the addition
 * @param number_2 number that is added to the first one
 */
static void add_numbers(char *number_1, char *number_2)
{
    char carry[] = {'0', '\0'};
    int offset = strlen(number_1) - strlen(number_2); //second number is smaller by that amount, we shift indices accordingly

    for (int i = strlen(number_1) - 1; i >= 0; i--) // we start at the end of the first number and stop after its first digit, which is at index 0
    {
        char b[2]; // b is the second hexdigit, we set it to 0 in case our number is smaller than i 
        b[1] = '\0';
        if (i - offset >= 0) // if b is big enough, store hexdigit at index i in it
        {
            b[0] = number_2[i - offset];
        }
        else
        {
            b[0] = '0';
        }
        char a[] = {number_1[i], '\0'}; // a is always large enough, so we can always grab the digit at index i

        unsigned int int_a = (int)strtol(a, NULL, 16); // we currently have the hexdigits stored as a char, so we convert them
        unsigned int int_b = (int)strtol(b, NULL, 16);
        unsigned int int_carry = (int)strtol(carry, NULL, 16);

        unsigned int result = int_a + int_b + int_carry; // add the result together in decimal

        int_carry = result / 16; // if our result is larger than 16 we have a carry over, else it is 0
        sprintf(carry, "%x", int_carry); // result is written back to carry as a hexdigit

        result = result % 16; // this is the result of the addition without the carry over

        char newChar[] = {' ', '\0'};
        sprintf(newChar, "%x", result); // we also convert it back to a hexdigit
        strncpy(&number_1[i], newChar, 1); // we put it into the number1 char array
    }

    if (strcmp(carry, "0") != 0) // in case our addition leaves a carry over at the end, we add that to the front of the char array
    {
        int length = strlen(number_1);
        for (int i = length; i >= 0; i--)
        {
            number_1[i + 1] = number_1[i]; // we do this by shifting all other numbers back by one
        }
        strncpy(&number_1[0], carry, 1); // and adding it to the front
    }
}

/**
 * @brief adds the correct amount of zeros to the numbers and then calls add_numbers.
 * add_numbers stores the result in the first number, so we call it 3 times. 
 * 
 * @param results char array that holds all our numbers
 * @param length length of the numbers that were originally read in getline, important to know how many zeros we add
 */
static void calc_result(char **results, size_t length)
{
    add_zeros(results[0], length);
    add_zeros(results[1], (length / 2));
    add_zeros(results[2], (length / 2));

    for (int i = 1; i < 4; i++)
    {
        add_numbers(results[0], results[i]);
    }
}


void USAGE(void)
{
    fprintf(stderr, "Usage: %s", PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief entry point of program
 * 
 * @details starts by getting the two numbers from stdin and checking their validity.
 * goes into base case if numbers are only one digit long.
 * if not it halfs the strings and forks the children, sends them the half strings.
 * we then wait for the children to complete, read their results and add them properly with calc_result.
 * we print the result and free the allocated memory.
 * 
 * @param argc argument count
 * @param argv argument vector
 * @return int exits with EXIT_FAILURE if hexadecimal addition fails, EXIT_SUCCESS otherwise
 */
int main(int argc, char *const *argv)
{
    PROGRAM_NAME = argv[0];

    if (argc > 1)
    {
        USAGE();
    }

    char *numbers[2];
    numbers[0] = NULL;
    numbers[1] = NULL;

    size_t length_1, buffer_length;
    if ((length_1 = getline(&(numbers[0]), &buffer_length, stdin)) < 0)
    {
        free(numbers[0]);
        free(numbers[1]);
        fprintf(stderr, "[%s] ERROR: Couldnt get first input.\n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    size_t length_2, buffer_length2;
    if ((length_2 = getline(&(numbers[1]), &buffer_length2, stdin)) < 0)
    {
        free(numbers[0]);
        free(numbers[1]);
        fprintf(stderr, "[%s] ERROR: Couldnt get second input.\n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }
    if (check_input(numbers[0]) < 0 || check_input(numbers[1]) < 0)
    {
        free(numbers[0]);
        free(numbers[1]);
        exit(EXIT_FAILURE);
    }

    if (length_1 != length_2)
    {
        free(numbers[0]);
        free(numbers[1]);
        fprintf(stderr, "[%s] ERROR: Both Hexadecimal Numbers have to have the same length\n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    if ((length_1 - 1) == 1) // recursion base case, converts the numbers from a char to int in base 16 and adds them
    {
        unsigned int a = (int)strtol(numbers[0], NULL, 16);
        unsigned int b = (int)strtol(numbers[1], NULL, 16);
        int result = a * b;
        fprintf(stdout, "%02x\n", result); // prints the result to stdout in the correct format, adds leading 0 if number has only 1 digit
        free(numbers[0]); 
        free(numbers[1]);
        exit(EXIT_SUCCESS);
    }

    fork_children(length_1 / 2, numbers);

    wait_children();

    char *results[4];

    get_results_children(results);

    calc_result(results, length_1 - 1);

    fprintf(stdout, "%s\n", results[0]);
    fflush(stdout);

    free(results[0]);
    free(results[1]);
    free(results[2]);
    free(results[3]);

    exit(EXIT_SUCCESS);
}