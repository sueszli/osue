/**
 * 
 * @file intmul.c
 * @author Alina Grassauer 11905176
 * @date 07.12.2021
 * @brief BSUE Exercise 2 intmul
 * @details 
 * 
 */

#include "intmul.h"

/**
 * @details name of the program (selbserklärend)
 * 
 */
static const char *PROGRAM_NAME;

/**
 * @details pids of the children; needs to be global for wait_for_children function
 * 
 */
static pid_t pids[CHILD_COUNT];

/**
 * @details firstinput hexnumber (needs to be global in order to free it at the end)
 * 
 */
static char *num_A;

/**
 * @details second input hexnumber (needs to be global in order to free it at the end)
 * 
 */
static char *num_B;

/**
 * @details pipe to read what children wrote to; needs to be global cause we need it in the
 * read_from_and_calc function
 * 
 */
static int pipe_out[CHILD_COUNT][2];

/**
 * @details pipe to write to the parent (could be local but since pipe_out is global, i decided
 * to make it global too for the sake of completeness; and i need it in multiple functions so it
 * is easier to access)
 * 
 */
static int pipe_in[CHILD_COUNT][2];

/**
 * @brief verifys if the numberes are hexdigits
 * 
 * @param length of the numbers
 * @return returns 1 for true and 0 for false
 */
static size_t verify_num(size_t length)
{

    for (size_t i = 0; i < length - 1; i++)
    {
        if ((isxdigit(num_A[i]) == 0) || (isxdigit(num_B[i]) == 0))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief error exit if an error occured
 * 
 * @param msg is the meassage which gets printed to stderr
 */
static void error_exit(char *msg)
{

    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

/**
 * @brief represents our termination function, which we need to cleanup (free)
 * our inputs; is used with atexit()
 * 
 */
static void term_func(void)
{

    if (num_A != NULL)
    {
        free(num_A);
    }

    if (num_B != NULL)
    {
        free(num_B);
    }
}

/**
 * @brief Gets the inputs written in stdin and checks if its correct
 * 
 * @return return length of the inputs insize_t; on error, program exits
 */
static size_t get_and_check_input(void)
{

    size_t buff_size = 0;
    char *input = NULL;
    size_t input_len1 = 0;
    ssize_t input_len = 0;

    while ((input_len = getline(&input, &buff_size, stdin)) != -1)
    {
        if (input_len1 == 0)
        {
            num_A = malloc(input_len + 1);
            strcpy(num_A, input);
            input_len1 = input_len;
        }
        else if (input_len1 == input_len)
        {
            num_B = malloc(input_len + 1);
            strcpy(num_B, input);
            break;
        }
        else
        {
            free(input);
            error_exit("not the same length");
        }
    }
    free(input);

    if ((input_len - 1 == 0) || (input_len == 0))
    {
        error_exit("no input");
    }

    if (num_A[input_len - 1] == '\n')
    {
        num_A[input_len - 1] = '\0';
    }
    if (num_B[input_len - 1] == '\n')
    {
        num_B[input_len - 1] = '\0';
    }

    if (verify_num(input_len) == 0)
    {
        error_exit("no hex number");
    }

    return input_len - 1;
}
/**
 * @brief gets a hex digit/char and converts it to an integer
 * 
 * @param hex is a hex digit/char
 * @return int is the hex digit in integer format
 */
static char dec_to_hex(int dec)
{

    char hex = ' ';

    if ((dec <= 15) && (dec >= 0))
    {
        if (dec < 10)
        {
            hex = '0' + dec;
        }
        else
        {
            hex = 'a' + dec - 10;
        }
    }
    return hex;
}

/**
 * @brief gets an integer and converts it to a hex digit/char
 * 
 * @param dec is the integer we want to convert
 * @return char is the result as a hex digit/integer
 */
static int hex_to_dec(char hex)
{

    int dec = -1;

    if (((hex >= '0') && (hex <= '9')))
    {
        dec = hex - '0';
    }
    else if (((hex >= 'a') && (hex <= 'f')))
    {
        dec = hex - 'a' + 10;
    }
    else if (((hex >= 'A') && (hex <= 'F')))
    {
        dec = hex - 'A' + 10;
    }
    return dec;
}

/**
 * @brief if both numbers are only one digit, the numbers are multiplied,
 * result gets send to stdout, exit with success
 * 
 */
static void end_of_reursion(void)
{

    int a = hex_to_dec(num_A[0]);
    int b = hex_to_dec(num_B[0]);

    fprintf(stdout, "%X\n", (a * b));
    fflush(stdout);

    exit(EXIT_SUCCESS);
}

/**
 * @brief closes unused ends
 * 
 * @param i represents the child/the corresponding pipes (position of the array)
 * @param end1 resprents an integer which is either the read end or the write end of pipe_in
 * @param end2 resprents an integer which is either the read end or the write end of pipe_out
 */
static void close_ends(int i, int end1, int end2)
{

    if (close(pipe_in[i][end1]) == -1 || close(pipe_out[i][end2]) == -1)
    {
        error_exit("closing pipes");
    }
}

/**
 * @brief redirects pipe ends of th children and closes unused pipe ends
 * 
 * @param i represents the child/the corresponding pipes (position of the array)
 *  */
static void child_process(int i)
{
    //close unused ends
    close_ends(i, WRITE_END, READ_END);

    if (dup2(pipe_in[i][READ_END], STDIN_FILENO) == -1 || dup2(pipe_out[i][WRITE_END], STDOUT_FILENO) == -1)
    {
        error_exit("pipe redirect");
    }

    close_ends(i, READ_END, WRITE_END);

    execlp(PROGRAM_NAME, PROGRAM_NAME, NULL);
    error_exit("new execution");
}

/**
 * @brief Creates a pipe and fork object, it calls methods for closing 
 * unused pipe ends
 * 
 */
static void create_pipe_and_fork(void)
{

    for (size_t i = 0; i < CHILD_COUNT; i++)
    {
        if (pipe(pipe_in[i]) == -1 || pipe(pipe_out[i]) == -1)
        {
            error_exit("couldn't pipe");
        }

        pids[i] = fork();

        switch (pids[i])
        {
        case -1:
            error_exit("couldn't fork");
            break;
        case 0: //child case
            child_process(i);
            break;
        default: // parent case
            close_ends(i, READ_END, WRITE_END);
            break;
        }
    }
}

/**
 * @brief writes the splittet input to the right pipe
 * 
 * @param first is splittet number A
 * @param second is splittet number B
 * @param i indicates the right write pipe we want to open
 * @param half_len indicates how much we want to send in printf (helps us split)
 */
static void write_to_file(char *first, char *second, int i, int half_len)
{
    FILE *write;

    if ((write = fdopen(pipe_in[i][WRITE_END], "w")) == NULL)
    {
        error_exit("opening write file");
    }
    fprintf(write, "%.*s\n", half_len, first);
    fprintf(write, "%.*s\n", half_len, second);

    if (fclose(write) != 0)
    {
        error_exit("closing write file");
    }
}

/**
 * @brief writes the splittet input to the right child processes
 * 
 * @param length is needed to split the input 
 */
static void write_to_child_process(int length)
{

    int half_len = length / 2;

    // "splits" input
    char *Ah = num_A;
    char *Al = num_A + half_len;
    char *Bh = num_B;
    char *Bl = num_B + half_len;

    for (size_t i = 0; i < CHILD_COUNT; i++)
    {
        // AhBh
        if (i == 0)
        {
            write_to_file(Ah, Bh, i, half_len);
        }
        // AhBl
        if (i == 1)
        {
            write_to_file(Ah, Bl, i, half_len);
        }
        // AlBh
        if (i == 2)
        {
            write_to_file(Al, Bh, i, half_len);
        }
        // AlBl
        if (i == 3)
        {
            write_to_file(Al, Bl, i, half_len);
        }
    }
}

/**
 * @brief reads from all four children, and saves them at the correct position 
 * (16^n/, 16^n/2, etc)in an array which has the size of the double length of the 
 * input (maximum lenght of out results from the children)
 * after that, it calculates the right value for each position of the array (summation)
 * and writes it to stdout
 * 
 * @param size represents the size of the original input before splitting
 * is neede to calculate the double size end the half size
 */
static void read_from_and_calc(int size)
{

    // stores the results from the children
    char *tmp_result[CHILD_COUNT];
    
    ssize_t input_len = 0;
    size_t child_results_len[CHILD_COUNT];

    char zero = '0';

    for (size_t i = 0; i < CHILD_COUNT; i++)
    {
        FILE *read = fdopen(pipe_out[i][READ_END], "r");

        if (read == NULL)
        {
            error_exit("opening read file");
        }
        size_t buff_size = 0;
        input_len = 0;
        tmp_result[i] = NULL;

        if ((input_len = getline(&tmp_result[i], &buff_size, read)) == -1)
        {
            for (size_t j = 0; j <= i; j++)
            {
                free(tmp_result[j]);
            }
            fclose(read);
            error_exit("child failed");
        }
        if (tmp_result[i][input_len - 1] == '\n')
        {
            tmp_result[i][input_len - 1] = '\0';
            input_len--;
        }

        child_results_len[i] = input_len;

        if (fclose(read) != 0)
        {
            error_exit("closing read file");
        }
    }

    // the following code resizes the different children and adds zeros

    int half_size = size / 2;
    int double_size = 2 * size;
    char tmp_zeros_result[4][double_size + 1];
    int pos[CHILD_COUNT] = {size, half_size, half_size, 0};
    int child_length[CHILD_COUNT];

    for (size_t i = 0; i < CHILD_COUNT; i++)
    {
        child_length[i] = child_results_len[i];
        memset(tmp_zeros_result[i], zero, double_size * sizeof(char));

        int amount = double_size - pos[i] - child_length[i];
        int k = 0;

        for (size_t j = amount; j < amount + child_length[i]; j++)
        {
            tmp_zeros_result[i][j] = tmp_result[i][k];
            k++;
        }

        tmp_zeros_result[i][double_size] = '\0';
    }

    // the following code adds up the four children

    int carry = 0;
    char result[double_size + 1];
    memset(result, zero, double_size * sizeof(char));
    result[double_size] = '\0';

    for (int i = double_size - 1; i >= 0; i--)
    {

        int albl = hex_to_dec(tmp_zeros_result[0][i]);
        int albh = hex_to_dec(tmp_zeros_result[1][i]);
        int ahbl = hex_to_dec(tmp_zeros_result[2][i]);
        int ahbh = hex_to_dec(tmp_zeros_result[3][i]);

        int tmp_res = albl + albh + ahbl + ahbh + carry;

        carry = tmp_res / 16;
        tmp_res = tmp_res % 16;

        result[i] = dec_to_hex(tmp_res);
    }

    for (size_t i = 0; i < CHILD_COUNT; i++)
    {
        free(tmp_result[i]);
    }

    fprintf(stdout, "%s\n", result);
    fflush(stdout);
}

/**
 * @brief waits for the children via theire pids, exits as exit failure if
 * one of the children exited on failure
 * 
 */
static void wait_for_children(void)
{

    int status;
    int c = 0;

    for (size_t i = 0; i < CHILD_COUNT; i++)
    {
        if (waitpid(pids[i], &status, 0) == -1)
        {
            error_exit("waiting pid");
        }

        if (WEXITSTATUS(status) != EXIT_SUCCESS)
        {
            c++;
        }
    }
    if (c > 0)
    {
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief the program gets two hex numbers and multiplies them by splitting them up
 * it uses fork and pipes
 * 
 * 
 * @param argc program counter
 * @param argv program arguments contain only the program name
 * @return int success or failure
 */
int main(int argc, char **argv)
{

    PROGRAM_NAME = argv[0];

    num_A = NULL;
    num_B = NULL;

    if (argc > 1)
    {
        fprintf(stderr, "Usage: %s \n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    if (atexit(term_func) < 0)
    {
        error_exit("termination function");
    }

    size_t input_len = get_and_check_input();

    if (input_len == 1)
    {
        end_of_reursion();
    }

    if (input_len % 2 != 0)
    {
        error_exit("odd number");
    }

    create_pipe_and_fork();

    write_to_child_process(input_len);

    wait_for_children();

    read_from_and_calc(input_len);

    exit(EXIT_SUCCESS);
}
