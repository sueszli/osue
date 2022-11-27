/**
 * @file intmul.c
 * @author Sejdefa Ibisevic <e11913116@student.tuwien.aca.at>
 * @brief
 * @version 0.1
 * @date 2021-12-04
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define CHILDREN 4                 // number of children
#define BUFFER_SIZE 64            // initial size of a buffer
static const char *program = NULL; // program name which will be set to argv[0]

static void error(char *error_message);
static void usage(void);
static char *get_input(FILE *in);
void hex_arg_validity(const char *hex);
void multiply_one_digit(const char *Abuff, const char *Bbuff);
static void send_data(int pipe_fd, const char *A, const char *B);
static char *power16(char *hex, int n);
static char dec_hex_convert(int i);
static int hex_dec_convert(char c);
static char *sum_hex(char *hex1, char *hex2);

/**
 * @brief It takes an error_meessage, prints it to stderr with program name and errno
 * and exits with EXIT_FAILURE
 * @param error_message error message to print to stderr
 */
static void error(char *error_message)
{
    fprintf(stderr, "[%s] ERROR: %s: %s.\n", program, error_message, strcmp(strerror(errno), "Success") == 0 ? "Failure" : strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Function to print corrcet version program input arguments
 *
 */
static void usage(void)
{
    fprintf(stderr, "Usage: %s \n", program);
}

/**
 * @brief Get the input data from a pointer provided as an argument
 * @details The function allocates the memory according to the BUFFER_SIZE.
 * It takes the data, character by character, from a pointer provided as an argument.
 * It stores the data into a buffer. The buffer is reallocated if the limit is reached.
 * Function returns the pointer to a buffer with stored input data.
 * @param in Pointer to an input file
 * @return char* Pointer to a buffer of containing input data
 */
static char *get_input(FILE *in)
{
    char *input;
    char c;
    int size = BUFFER_SIZE;
    int len = 0;

    if ((input = malloc(BUFFER_SIZE)) == NULL)
    {
        error("alloc failed at get_input");
    }

    while ((c = fgetc(in)) != EOF && c != '\n')
    {
        input[len++] = c;
        if (len == size)
        {
            if ((input = realloc(input, sizeof(char) * (size <<= 1))) == NULL)
                error("realloc failed at get_input");
        }
    }
    input[len++] = '\0';

    return input;
}

/**
 * @brief Checks if the digits are in the hexadecimal range
 * @details Fuction takes a pointer on a hexadecimal array.
 * It checks if each character of a given array is in range of 0-9 or A-F
 * In case of invalid character, error is produced.
 * @param hex Pointer to an array of hexadecimal characters
 */
void hex_arg_validity(const char *hex)
{
    int valid = 0;

    for (int i = 0; i < strlen(hex); i++)
    {

        if ((hex[i] >= '0' && hex[i] <= '9') ||
            (hex[i] >= 'A' && hex[i] <= 'F') ||
            (hex[i] >= 'a' && hex[i] <= 'f'))
        {
            valid = 1;
        }
        else
        {
            valid = 0;
        }
        if (valid == 0)
        {
            error("not a hexadecimal symbol found");
        }
    }
}

/**
 * @brief Function that multiplys one-digit hexadecimal numbers
 * @details Function accepts two pointers on a  one-digit hexadecimal numbers.
 * It converts characters to a corresponding digital number using strtol(3).
 * It prints the result on stdout as hexadecimal.
 * @param Abuff First number to be multiplied
 * @param Bbuff Second number to be multiplied.
 */
void multiply_one_digit(const char *Abuff, const char *Bbuff)
{
    int a, b;

    a = strtol(Abuff, NULL, 16);
    b = strtol(Bbuff, NULL, 16);

    printf("%x\n", a * b);

    fflush(stdout);
    exit(EXIT_SUCCESS);
}

/**
 * @brief Send data to children process
 * @details Function opens a pipe decriptor as a write. It writes the data provided as an argument to a pipe.
 * It closes opened pipe.
 * @param pipe_fd pipe descriptor
 * @param A Pointer to a first hexadecimal number
 * @param B Pointer to a second hexadecimal number
 */
static void send_data(int pipe_fd, const char *A, const char *B)
{

    // open pipe descriptor
    FILE *out = fdopen(pipe_fd, "w");

    if (out == NULL)
    {
        error("failed to open pipe out fd");
    }
    // write data to pipe
    if (fprintf(out, "%s\n%s\n", A, B) < 0)
    {
        error("failed to write data to child");
    }
    fflush(out);
    if (fclose(out) == EOF)
    {
        error("failed to close out");
    }
}

/**
 * @brief It multiplies hexadecimal number hex with 16^n
 * @details Function takes pointer to a hexadecimal number hex as an argument and the power of 16, n.
 * It allocates memory for a result as current length of hex + n.
 * It adds n zeros to the right side of hex, or it does left shift for n places.
 * The shifted hexadecimal number is returned as a result.
 * @param hex Pointer to a hexadecimal number to be multiplyed by n power of 16
 * @param n Power of 16
 * @return char* Pointer to a result of shifting/multiplying hexadecimal number
 */
static char *power16(char *hex, int n)
{
    // shifts the string hex for n values to the left
    char *result = (char *)malloc(strlen(hex) + n + 1);
    memset(result, 0, strlen(hex) + n + 1);
    strcpy(result, hex);

    // add n '0's to the end
    for (int i = 0; i < n; i++)
    {
        strcat(result, "0");
    }

    return result;
}

/**
 * @brief Converts integer to hexadecimal character
 * @details Function takes an integer as an argument and retuns corresponding value in hexadecimal range.
 * @param i Integer to be converted
 * @return char Character of an integer in hexadecimal range
 */
static char dec_hex_convert(int i)
{

    if (i >= 0 && i <= 9)
    {
        return (i + '0');
    }
    else if (i >= 10 && i <= 15)
    {
        return (i + 'a' - 10);
    }
    else
    {
        error("tryintg to convert invalid integer to hexadecimal");
        return -1;
    }
}

/**
 * @brief Converts hexadecimal number to a integer
 * @details Function takes hexadecimal character and returns the corresponding value in decimal range.
 * @param c Character to be converted into integer
 * @return int Integer value of character
 */
static int hex_dec_convert(char c)
{
    if (c >= '0' && c <= '9')
    {
        return (c - '0');
    }
    else if (c >= 'a' && c <= 'f')
    {
        return (c - 'a' + 10);
    }
    else if (c >= 'A' && c <= 'F')
    {
        return (c - 'A' + 10);
    }
    else
    {
        error("tryintg to convert invalid hexadecimal character to integer");
        return -1;
    }
}

/**
 * @brief Adds two hexadecimal numbers and returns pointer on a result
 * @details Function takes character by character of each hexadecimal number from right to left.
 * It converts the characters into their integer value. Then, values are added and if sum exceeds value of 15, carry is increased.
 * Function also adds zeros on left side of hexadecimal strings, or does right shift.
 * It returns pointer on a resulting buffer.
 * @param hex1 First hexadecimal number to be added
 * @param hex2 Second hexadecimal number to be added
 * @return char* Pointer to a resulting buffer after addition
 */
static char *sum_hex(char *hex1, char *hex2)
{
    char *out = (char *)malloc(BUFFER_SIZE + 5);
    int buff_len = BUFFER_SIZE;
    if (out == NULL)
        error("failed to malloc buffer in sum_hex");
    memset(out, 0, BUFFER_SIZE + 5);
    int iter = 0;
    int carry = 0; // carry bit for sum over 16

    int len1 = strlen(hex1);
    int len2 = strlen(hex2);
    int index1 = len1 - 1;
    int index2 = len2 - 1;
    int greater = len1 > len2 ? len1 : len2;

    while (greater > 0)
    {

        // sums from right to left
        char c1, c2;
        // add zeros to right - right shift
        if (index1 >= 0)
        {
            c1 = hex1[index1--];
        }
        else
        {
            c1 = '0';
        }

        if (index2 >= 0)
        {
            c2 = hex2[index2--];
        }
        else
        {
            c2 = '0';
        }

        greater--;

        // get decimal value of characters
        int val1 = hex_dec_convert(c1);
        int val2 = hex_dec_convert(c2);

        int sum = val1 + val2 + carry;

        if (sum >= 16)
            carry = 1;
        else
            carry = 0;

        sum %= 16;

        if (iter == buff_len)
        {
            // reallocate out buffer, left shift by one
            out = realloc(out, (buff_len <<= 1) + 5);
            memset(out + iter, 0, 4);
            if (out == NULL)
            {
                error("failed to realloc buffer in sum_hex");
            }
        }
        // write hex value in buffer
        out[iter++] = dec_hex_convert(sum);
    }

    // add 1 for final carry
    if (carry == 1)
        out[iter++] = '1';

    // buffer content is in opposite order and need sto be reversed
    int length = strlen(out);
    for (int i = 0; i < (int)(length / 2); i++)
    {
        char temp = out[i];
        out[i] = out[length - i - 1];
        out[length - i - 1] = temp;
    }

    out[iter] = '\0';

    return out;
}

/**
 * @brief Main function that does multiplication of two hexadecimal number
 * @details It takes two hexadecimal numbers from stdin. Checks if the numbers are valid, equal lenght and not NULL.
 * If the numbers are one digit, it does one digit multiplication. If numbers contain more then one digit then both strings
 * are evenly split into Ah, Al, Bh, Bl. It allocates two pipes per child for inter-process communication.
 * Finally, the process forks recursively into four children processes.
 * @param argc Number of arguments provided
 * @param argv Array of arguments
 * @return int EXIT_SUCCESS
 */
int main(int argc, char **argv)
{

    program = argv[0];

    if (argc > 1)
    {
        usage();
        error("invalid number of arguments");
    }

    char *Abuff = get_input(stdin);
    char *Bbuff = get_input(stdin);

    if (Abuff == NULL || Bbuff == NULL)
    {
        error("unable to read data from stdin");
    }

    int A_len = strlen(Abuff);
    int B_len = strlen(Bbuff);

    if (A_len == 0 || B_len == 0)
    {
        error("number not given");
    }

    // check if they are equal length
    if (A_len != B_len)
    {
        error("A and B are not equal length");
    }

    // replace new line with 0 termination
    if (A_len > 0 && Abuff[A_len - 1] == '\n')
    {
        Abuff[A_len - 1] = '\0';
        A_len--;
    }

    if (B_len > 0 && Bbuff[B_len - 1] == '\n')
    {
        Bbuff[B_len - 1] = '\0';
        B_len--;
    }

    // multiplication of one-digit hexadecimals
    if (A_len == 1 && B_len == 1)
    {
        multiply_one_digit(Abuff, Bbuff);
    }

    // check if numbers have even number of digits
    if ((A_len % 2) != 0 || (B_len % 2) != 0)
    {
        error("number of decimals is not even");
    }

    if ((A_len & (A_len-1)) != 0) {
        error("input length is not a power of two");
    }

    // check if Abuff and Bbuff are hexadecimal
    hex_arg_validity(Abuff);
    hex_arg_validity(Bbuff);

    // two pipes for each child
    int pipe_out_fd[2 * CHILDREN];
    int pipe_in_fd[2 * CHILDREN];
    pid_t pids[CHILDREN] = {0};
    int n = A_len;
    int n_half = n / 2;

    // initializing pipe
    for (int i = 0; i < CHILDREN; i++)
    {
        if (pipe(&pipe_out_fd[2 * i]) < 0 || pipe(&pipe_in_fd[2 * i]) < 0)
        {
            error("failed to create pipes");
        }
    }

    // data is divided into Ah, Al, Bh, Bl parts, one for each child process
    char Ah[n_half + 1];
    memset(Ah, 0, n_half + 1);
    strncpy(Ah, Abuff, n_half);

    char Al[n_half + 1];
    memset(Al, 0, n_half + 1);
    strncpy(Al, &Abuff[n_half], n_half);

    char Bh[n_half + 1];
    memset(Bh, 0, n_half + 1);
    strncpy(Bh, Bbuff, n_half);

    char Bl[n_half + 1];
    memset(Bl, 0, n_half + 1);
    strncpy(Bl, &Bbuff[n_half], n_half);

    free(Abuff);
    free(Bbuff);

    // send data go pipes and forks
    for (int i = 0; i < CHILDREN; i++)
    {
        char *A = NULL;
        char *B = NULL;

        if (i == 0)
        {
            A = Ah;
            B = Bh;
        }
        else if (i == 1)
        {
            A = Ah;
            B = Bl;
        }
        else if (i == 2)
        {
            A = Al;
            B = Bh;
        }
        else if (i == 3)
        {
            A = Al;
            B = Bl;
        }

        send_data(pipe_out_fd[2 * i + 1], A, B);
        pids[i] = fork();
        if (pids[i] < 0)
        {
            error("fork failed");
        }
        else if (pids[i] == 0)
        {
            // child process
            // override stdin, stdout fd
            if (dup2(pipe_in_fd[2 * i + 1], STDOUT_FILENO) < 0)
            {
                error("failed to dup2 fd out");
            }

            if (dup2(pipe_out_fd[2 * i], STDIN_FILENO) < 0)
            {
                error("failed to dup2 fd in");
            }

            // recursive execution
            if (execlp(program, program, NULL) < 0)
            {
                error("execlp failed");
            }
            assert(0);
        }

        close(pipe_in_fd[2 * i + 1]);
        close(pipe_out_fd[2 * i]);

    }

    // parent process
    char multiply[4][2 * n_half + 1];
    memset(multiply, 0, sizeof(multiply[0][0]) * 4 * (n_half * 2 + 1));

    // read child data to multiply
    for (int i = 0; i < CHILDREN; i++)
    {
        FILE *in;
        in = fdopen(pipe_in_fd[2 * i], "r");

        if (in == NULL)
        {
            error("failed to open pipe in fd");
        }
        if (fgets(multiply[i], n_half * 2 + 1, in) == NULL)
        {
            error("failed to read from child");
        }
        fclose(in);
    }

    // replace new line with 0 termination
    for (int i = 0; i < CHILDREN; i++)
    {
        int len = strlen(multiply[i]);
        if (multiply[i][len - 1] == '\n')
        {
            multiply[i][len - 1] = '\0';
            len--;
        }
    }

    // base 16 left shift to multiply by 16^n or 16^n_half
    char *result_1 = power16(multiply[0], n);      // Ah*Bh*16^n
    char *result_2 = power16(multiply[1], n_half); // Ah*Bl*16^n_half
    char *result_3 = power16(multiply[2], n_half); // Al*Bh*16^n_half
    char *result_4 = multiply[3];

    // summ results
    char *sum_1 = sum_hex(result_1, result_2);
    char *sum_2 = sum_hex(sum_1, result_3);
    char *sum_3 = sum_hex(sum_2, result_4);

    // leading zero added if result has odd digit number
    int sum_3_len = strlen(sum_3);
    if ((sum_3_len % 2) != 0)
    {
        if ((sum_3 = realloc(sum_3, sum_3_len + 2)) == NULL)
            error("realloc failed in adding zero");
        strcpy(sum_3 + 1, sum_3);
        sum_3[0] = '0';
        sum_3[sum_3_len + 1] = '\0';
    }

    // print to stdout
    strcat(sum_3, "\n");
    if (fputs(sum_3, stdout) == EOF)
    {
        error("writing to stdout failed");
    }
    //printf("%s\n", sum_3);

    free(result_1);
    free(result_2);
    free(result_3);
    free(sum_1);
    free(sum_2);
    free(sum_3);

    // children termination
    for (int i = 0; i < CHILDREN; i++)
    {
        int status = 0;
        waitpid(pids[i], &status, 0);
        if (WEXITSTATUS(status) != EXIT_SUCCESS)
            error("waitpid failed");
    }

    exit(EXIT_SUCCESS);
}
