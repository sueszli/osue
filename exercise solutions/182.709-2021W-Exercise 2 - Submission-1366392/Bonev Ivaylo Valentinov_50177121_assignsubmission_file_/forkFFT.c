/**
 * @author 	Ivaylo Bonev, matriculation number: 12025894
 * @date 	11-Dec-2021
 * @brief 	Calculates the Fast-Fourier-Transform of an array of float values.
 * @details	Using the algorithm from the exercise, calculates the Fourier Transform
 *          of the passed values by continiously calling fork() and exec() for parallel
 *          processing and communicating between parent and child processes via pipes.
**/

//Standard libraries
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Library for errno
#include <errno.h>

//Library for math functions
#include <math.h>

//Libraries for forking and pipes
#include <fcntl.h>
#include <unistd.h>

//Library for wait()
#include <sys/wait.h>

//Custom library
#include "common.h"

//Name of the program (i.e. argv[0]); used in error messages.
const char* programName = "forkFFT";

/**
 * @brief   Exits the program and prints an error message.
 * @details Exits the program with EXIT_FAILURE, printing an error message to stderr.
 *          Also specifies the name of the program.
 *          DOES NOT free allocated resources!
 * @param msg   The error message to be printed to stderr.
**/
static void error_exit(const char* msg)
{
    fprintf(stderr, "[%s] ERROR: %s\n", programName, msg);
    exit(EXIT_FAILURE);
}

/**
 * @brief   Outputs the program's usage (synopsis) to the user.
 * @details Prints the program's synopsis to stderr.
 *          DOES NOT exit the program!
**/
static void usage(void)
{
    fprintf(stderr, "Usage: ./%s\n", programName);
}

/**
 * @brief   Frees all resources that were malloc'd or realloc'd.
 * @details Frees all resources that required manual memory allocation during the course of the program.
 *          Also called after every handled error, as well as on normal program termination.
**/
static void free_resources(void)
{
    //See common.h for more information on the variables used here.

    free(args.numbers);
    free(PE);
    free(PO);
    free(result);
    free(resultE);
    free(resultO);
}

/**
 * @brief   Reads the input of stdin line by line and converts each line to a float number.
 * @details Reads each line of stdin (until EOF is reched) and converts each line to a float number.
 *          Converted numbers are stored in a global args variable (see common.h/args_t for more information).
**/
static void read_arguments(void)
{
    char* line = (char*) malloc(maxLineLength * sizeof(char));
    if (line == NULL)
    {
        free_resources();
        error_exit("Could not allocate enough memory for reading the lines!");
    }

    size_t charCount;
    int index = 0;
    char* remainingStr;
    while ((charCount = getline(&line, &maxLineLength, stdin)) != -1) {

		//Convert each line to a float number and store it in the arguments array.
        //If any number is in a wrong format, throw an error.

        args.numbers = realloc(args.numbers, (index + 1) * sizeof(float));
        if (args.numbers == NULL)
        {
            free(line);
            free_resources();
            error_exit("Could not allocate enough memory for parsing the input!");
        }

		errno = 0;
        args.numbers[index] = strtof(line, &remainingStr);
        if (errno != 0 || strcmp(remainingStr, "\n") != 0)
        {
            free(line);
            free_resources();
            error_exit("Number was not in a correct format!");
        }

        index++;
	}
    free(line);
    args.size = index;

    //An odd number of elements (apart from 1) is not allowed!
    if (args.size != 1 && args.size % 2 == 1)
    {
        free_resources();
        error_exit("Please enter an even number of elements! Cannot perform calculations with an odd number of points!");
    }
}

/**
 * @brief   Splits the passed array into two – one containing all even-indexed elements,
 *          and the other containing all odd-indexed elements.
 * @details Splits the passed number array into two parts – one containing all numbers that were at an even index,
 *          and a second part containing all numbers that were at an odd index.
 *          For example, the array {1, 2, 3, 4} would be split in "even" = {1, 3} and "odd" = {2, 4}.
 *          The two split parts are then written in the passed parameters "even" and "odd".
 *          It is assumed that they contain enough allocated memory for all numbers!
 * @param numbers   The array of numbers that is to be split. Function will exit with an error if numbers is NULL.
 * @param length    The length of the array "numbers". Function will exit with an error if length is negative.
 * @param even      The (already allocated!) array where the even-indexed part of "numbers" will be stored.
 *                  Function will exit with an error if even is NULL.
 * @param even      The (already allocated!) array where the odd-indexed part of "numbers" will be stored.
 *                  Function will exit with an error if odd is NULL.
**/
static void split_in_odds_and_evens(float* numbers, int length, float* even, float* odd)
{
    if (numbers == NULL || even == NULL || odd == NULL)
    {
        free_resources();
        error_exit("Cannot split an empty array!");
    }

    if (length < 0)
    {
        free_resources();
        error_exit("Length cannot be negative!");
    }

    int evenInd = 0;
    int oddInd = 0;
    for (int i = 0; i < length; i++)
    {
        if (i % 2 == 0)
        {
            even[evenInd] = numbers[i];
            evenInd++;
        }
        else
        {
            odd[oddInd] = numbers[i];
            oddInd++;
        }
    }
}

/**
 * @brief   Returns the sum of two complex numbers.
 * @details Uses the formula that is commented inside of the function to add two complex numbers together.
 * @param num1  First complex number.
 * @param num2  Second complex number.
 * @return  The arithmetic sum of the two passed complex numbers as a new complex number.
**/
static complex_t add_complex(complex_t num1, complex_t num2)
{
    complex_t product;

    //(a + ib) + (c + id) = (a + c) + i(b + d)

    product.real = num1.real + num2.real;
    product.imaginary = num1.imaginary + num2.imaginary;

    return product;
}

/**
 * @brief   Returns the product of two complex numbers.
 * @details Uses the formula that is commented inside of the function to multiply two complex numbers together.
 * @param num1  First complex number.
 * @param num2  Second complex number.
 * @return  The arithmetic product of the two passed complex numbers as a new complex number.
**/
static complex_t mult_complex(complex_t num1, complex_t num2)
{
    //Used the formula from the exercise.
    complex_t product;

    //a*c - b*d
    product.real = (num1.real * num2.real) - (num1.imaginary * num2.imaginary);

    //i * (a*d + b*c)
    product.imaginary = (num1.real * num2.imaginary) + (num1.imaginary * num2.real);

    return product;
}

/**
 * @brief   Calculates the Fourier Transform of the float values that were passed to the program
 *          using the Fast-Fourier-Transform algorithm given in the exercise.
 * @details Uses the "butterfly" operation described in the exercise on the two half-results that were calculated
 *          by the children of this process (resultE and resultO; see common.h for more information).
 *          Also uses a custom PI value (see common.h/PI).
 *          Exits with an error if the global variables resultE or resultO are NULL.
 * @return  An array of complex numbers that are the result of the Fourier Transformation.
**/
static complex_t* calc_FFT(void)
{
    complex_t bracketsExprCompl;

    result = (complex_t*)malloc(args.size * sizeof(complex_t));
    if (result == NULL) 
    {
        free_resources();
        error_exit("Could not allocate enough memory for calculating the array result!");
    }

    if (resultE == NULL || resultO == NULL)
    {
        free_resources();
        error_exit("RE and RO are null! Can not calculate the Fourier Transform of the array!");
    }

    float tempValue;
    for (int i = 0; i < args.size; i++)
    {
        //Calculate this beforehand for better performance.
        tempValue = ((-2 * PI) / args.size) * i;
        bracketsExprCompl.real = cosf(tempValue);
        bracketsExprCompl.imaginary = sinf(tempValue);

        if (i < args.size / 2)
        {
            //First part of the formula for R
            result[i] = add_complex(resultE[i], mult_complex(bracketsExprCompl, resultO[i]));
        }
        else
        {
            //Second part of the formula for R.
            result[i] = add_complex(resultE[i - (args.size / 2)], mult_complex(bracketsExprCompl, resultO[i - (args.size / 2)]));
        }
    }

    return result;
}

/**
 * @brief   Outputs an array of complex numbers to the user, one number per line.
 * @details Prints each complex number in the array to the specified output stream.
 *          Each line is a single complex number in the format "a.b c.d*i"
 *          (e.g. 1.0 2.0*i, representing the complex number (1 + 2*i)).
 * @param numbers       The array of complex numbers. Function exits with an error if NULL!
 * @param length        The length of the array. Function exits with an error if negative!
 * @param outputStream  The output stream used for printing the numbers. Function exits with an error if NULL!
**/
static void print_complex_numbers(complex_t* numbers, int length, FILE* outputStream)
{
    if (numbers == NULL)
    {
        free_resources();
        error_exit("Cannot print empty numbers array!");
    }
    if (length < 0)
    {
        free_resources();
        error_exit("Length cannot be negative!");
    }
    if (outputStream == NULL)
    {
        free_resources();
        error_exit("Output stream cannot be NULL!");
    }

    for (int i = 0; i < length; i++)
    {
        fprintf(outputStream, "%f %f*i\n", numbers[i].real, numbers[i].imaginary);
    }
}

/**
 * @brief   Outputs an array of float numbers to the user, one number per line.
 * @details Prints each number in the array to the specified output stream.
 *          Each line is a single float number in the format "a.b" (e.g. 1.0, 2.545).
 * @param numbers       The array of float numbers. Function exits with an error if NULL!
 * @param length        The length of the array. Function exits with an error if negative!
 * @param outputStream  The output stream used for printing the numbers. Function exits with an error if NULL!
**/
static void print_floats(float* numbers, int length, FILE* outputStream)
{
    if (numbers == NULL)
    {
        free_resources();
        error_exit("Cannot print empty numbers array!");
    }
    if (length < 0)
    {
        free_resources();
        error_exit("Length cannot be negative!");
    }
    if (outputStream == NULL)
    {
        free_resources();
        error_exit("Output stream cannot be NULL!");
    }

    for (int i = 0; i < length; i++)
    {
        fprintf(outputStream, "%f\n", numbers[i]);
    }
}

/**
 * @brief   Creates the necessary pipes and stores them in the pipe variables defined in common.h.
 * @details Creates 4 pipes – a stdin pipe and a stdout for each child.
 *          Stores them in the pipe variables defined in common.h.
 *          Exits with an error if any pipe creation failed.
**/
static void init_pipes(void)
{
    //Create 2 pipes per child: 1 for stdin and 1 for stdout.
    if (pipe(parent_from_child1_stdin_pipe) < 0)
    {
        free_resources();
        error_exit("Could not create stdin pipe for child 1!");
    }
    if (pipe(parent_to_child1_stdout_pipe) < 0)
    {
        free_resources();
        error_exit("Could not create stdout pipe for child 1!");
    }
    if (pipe(parent_from_child2_stdin_pipe) < 0)
    {
        free_resources();
        error_exit("Could not create stdin pipe for child 2!");
    }
    if (pipe(parent_to_child2_stdout_pipe) < 0)
    {
        free_resources();
        error_exit("Could not create stdout pipe for child 2!");
    }
}

/**
 * @brief   Closes unused pipe ends and duplicates the necessary file descriptors. Only relevant when in a child process.
 * @details Closes the necessary pipe ends so that the child can read from the stdout of the parent in stdin,
 *          and output to the stdin of the parent in stdout.
 *          This also requires duplication of some of the pipe ends (using dup2()).
 *          Exits with an error if any of the operations (pipe closing, duplicating, etc) failed.
 * @param child_from_parent_stdin_pipe  The pipe in direction parent-child that is used by the child to read
 *                                      what the parent wrote to stdout in stdin.
 *                                      Function exits with an error if NULL.
 * @param child_to_parent_stdout_pipe   The pipe in direction child-parent that is used by the child to write
 *                                      to stdout what the parent should read in stdin.
 *                                      Function exits with an error if NULL.
**/
static void manage_child_pipe_ends(int child_from_parent_stdin_pipe[], int child_to_parent_stdout_pipe[])
{
    //NOTE:
    //Pipe roles are reversed between children and parents!
    //A parent's stdout pipe is a child's stdin pipe (the parent writes to stdout, the child reads from stdin)!
    //Hence the silly variable names...

    if (child_from_parent_stdin_pipe == NULL || child_to_parent_stdout_pipe == NULL)
    {
        error_exit("Can not manage empty pipes!");
    }

    //Using the diagramm from the lecture for reference,
    //close the inactive pipe ends and duplicate the opened ones.

    if (close(child_from_parent_stdin_pipe[WRITE_END]) < 0)
    {
        free_resources();
        error_exit("Could not close inactive stdin pipe end of child!");
    }
    if (dup2(child_from_parent_stdin_pipe[READ_END], STDIN_FILENO) < 0)
    {
        free_resources();
        error_exit("Could not duplicate stdin of child!");
    }
    if (close(child_from_parent_stdin_pipe[READ_END]) < 0) //necessary after dup2!
    {
        free_resources();
        error_exit("Could not close stdin pipe end of child after duplicating it!");
    }

    if (close(child_to_parent_stdout_pipe[READ_END]) < 0)
    {
        free_resources();
        error_exit("Could not close inactive stdout pipe end of child!");
    }
    if (dup2(child_to_parent_stdout_pipe[WRITE_END], STDOUT_FILENO) < 0)
    {
        free_resources();
        error_exit("Could not duplicate stdout of child!");
    }
    if (close(child_to_parent_stdout_pipe[WRITE_END]) < 0) //necessary after dup2!
    {
        free_resources();
        error_exit("Could not close stdout pipe end of child after duplicating it!");
    }
}

/**
 * @brief   Closes unused pipe ends. Only relevant when in a parent process.
 * @details Closes the necessary pipe ends so that the parent can read from the stdout of the child in stdin,
 *          and output to the stdin of the child in stdout.
 *          Exits with an error if any of the operations failed.
 * @param parent_from_child_stdin_pipe  The pipe in direction child-parent that is used by the parent to read
 *                                      what the child wrote to stdout in stdin.
 *                                      Function exits with an error if NULL.
 * @param parent_to_child_stdout_pipe   The pipe in direction parent-child that is used by the parent to write
 *                                      to stdout what the child should read in stdin.
 *                                      Function exits with an error if NULL.
**/
static void manage_parent_pipe_ends(int parent_from_child_stdin_pipe[], int parent_to_child_stdout_pipe[])
{
    if (parent_from_child_stdin_pipe == NULL || parent_to_child_stdout_pipe == NULL)
    {
        error_exit("Can not manage empty pipes!");
    }

    //Using the diagramm from the lecture for reference,
    //close the inactive pipe ends.

    if (close(parent_from_child_stdin_pipe[WRITE_END]) < 0)
    {
        free_resources();
        error_exit("Could not close inactive stdin pipe end of parent!");
    }
    
    if (close(parent_to_child_stdout_pipe[READ_END]) < 0)
    {
        free_resources();
        error_exit("Could not close inactive stdout pipe end of parent!");
    }
}

/**
 * @brief   Reads from the specified input stream and converts what was read to an array of complex numbers.
 * @details Reads from the specified input stream line-by-line (until EOF) and attempts to convert each line
 *          to a complex number.
 *          Assumed each line is a single complex number in the format "a.b c.d*i"
 *          (e.g. 1.0 2.0*i, representing the complex number (1 + 2*i)).
 *          Exits with an error if any number was not in the correct format.
 * @param input_stream  The stream that the numbers are read from. Function exits with an error if NULL.
 * @param result        The (pre-allocated!) array where the read (and parsed) complex nubmers should be stored.
 *                      Function exits with an error if NULL.
 * @param inputLength   The amount of numbers that are expected in the input stream.
 *                      Function exits with an error if negative.
**/
static void read_complex_numbers(FILE* input_stream, complex_t* result, int inputLength)
{
    if (input_stream == NULL)
    {
        free_resources();
        error_exit("Input stream cannot be null!");
    }
    if (result == NULL)
    {
        free_resources();
        error_exit("Cannot output read complex numbers to an empty array!");
    }
    if (inputLength < 0)
    {
        free_resources();
        error_exit("Length cannot be negative!");
    }

    char* line = (char*)malloc(maxLineLength * sizeof(char));
    size_t charCount;
    char* remainingStr;
    char* remainingStr2;
    int index = 0;
    complex_t num;
    while ((charCount = getline(&line, &maxLineLength, input_stream)) != -1) {

        //Each line is a complex number in the format (float_number float_number*i)

		errno = 0;
        num.real = strtof(line, &remainingStr);

        //If there were only 2 numbers in the input, then each child will have returned a single float value,
        //and NOT a complex number.
        //This requires different parsing.
        if (inputLength == 2)
        {
            num.imaginary = 0.0f;
        }
        else
        {
            num.imaginary = strtof(remainingStr, &remainingStr2);
            if (errno != 0 || strcmp(remainingStr2, "*i\n") != 0)
            {
                free(line);
                free_resources();
                error_exit("Could not read complex number. Perhaps it was in a wrong format?");
            }
        }

        result[index] = num;
        index++;
	}

    free(line);
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        //No options or positional arguments are accepted!
        usage();
        exit(EXIT_FAILURE);
    }

    read_arguments();

    PE = (float*)malloc((args.size / 2) * sizeof(float));
    if (PE == NULL)
    {
        free_resources();
        error_exit("Could not allocate enough memory for the even indizes of the array!");
    }

    PO = (float*)malloc((args.size / 2) * sizeof(float));
    if (PO == NULL)
    {
        free_resources();
        error_exit("Could not allocate enough memory for the odd indizes of the array!");
    }

    //Create pipes (2 per child, 4 in total).
    init_pipes();

    //Split input and fork only if the input is > 1
    if (args.size > 1)
    {
        split_in_odds_and_evens(args.numbers, args.size, PE, PO);
        pid_t child_1_PID = fork();
        switch (child_1_PID)
        {
            case -1:
                free_resources();
                error_exit("Could not fork the program!");
            case 0:
                //Child 1

                manage_child_pipe_ends(parent_to_child1_stdout_pipe, parent_from_child1_stdin_pipe);

                //Start the program.
                execlp("./forkFFT", "forkFFT", NULL);

                //Shouldn't be reached.
                free_resources();
                error_exit("Could not execute program in child 1 process!");
            default:
                //Parent process.

                manage_parent_pipe_ends(parent_from_child1_stdin_pipe, parent_to_child1_stdout_pipe);

                //Write split array to child 1's stdin.
                FILE* pipeWriteEnd = fdopen(parent_to_child1_stdout_pipe[WRITE_END], "w");
                if (pipeWriteEnd == NULL)
                {
                    free_resources();
                    error_exit("Could not open parent-child1 stdout pipe end!");
                }

                print_floats(PE, args.size / 2, pipeWriteEnd);
                if (fclose(pipeWriteEnd) != 0)
                {
                    free_resources();
                    error_exit("Could not close parent-child1 stdout pipe end after writing to child!");
                }

                break;
        }

        //Still parent process
        pid_t child_2_PID = fork();

        switch (child_2_PID)
        {
            case -1:
                free_resources();
                error_exit("Could not fork the program!");
            case 0:
                //Child 2
                
                manage_child_pipe_ends(parent_to_child2_stdout_pipe, parent_from_child2_stdin_pipe);

                //Start the program.
                execlp("./forkFFT", "forkFFT", NULL);

                //Shouldn't be reached.
                free_resources();
                error_exit("Could not execute program in child 2 process!");
            default:
                //Parent process.
                manage_parent_pipe_ends(parent_from_child2_stdin_pipe, parent_to_child2_stdout_pipe);

                //Write split array to child 2's stdin.
                FILE* pipeWriteEnd = fdopen(parent_to_child2_stdout_pipe[WRITE_END], "w");
                if (pipeWriteEnd == NULL)
                {
                    free_resources();
                    error_exit("Could not open parent-child2 stdout pipe end!");
                }

                print_floats(PO, args.size / 2, pipeWriteEnd);
                if (fclose(pipeWriteEnd) != 0)
                {
                    free_resources();
                    error_exit("Could not close parent-child2 stdout pipe end after writing to child!");
                }

                break;
        }

        //Still parent process

        //Wait for ALL children to exit.
        //Even if a child exited with an error, wait for the rest of the children to terminate before exiting
        //in order to avoid leaving orphan processes.
        int status1;
        int status2;

        if (waitpid(child_1_PID, &status1, 0) < 0)
        {
            free_resources();
            error_exit("Could not wait for child 1!");
        }
        if (waitpid(child_2_PID, &status2, 0) < 0)
        {
            free_resources();
            error_exit("Could not wait for child 2!");
        }

        if (WEXITSTATUS(status1) != EXIT_SUCCESS)
        {
            free_resources();
            error_exit("Child 1 did not exit with EXIT_SUCCESS!");
        }
        if (WEXITSTATUS(status2) != EXIT_SUCCESS)
        {
            free_resources();
            error_exit("Child 2 did not exit with EXIT_SUCCESS!");
        }

        //Allocate the memory before reading the children's results.
        resultE = (complex_t*)malloc((args.size / 2) * sizeof(complex_t));
        if (resultE == NULL)
        {
            free_resources();
            error_exit("Could not allocate enough memory for reading output of child 1!");
        }
        resultO = (complex_t*)malloc((args.size / 2) * sizeof(complex_t));
        if (resultO == NULL)
        {
            free_resources();
            error_exit("Could not allocate enough memory for reading output of child 2!");
        }

        //Read first result from child 1's stdout.
        FILE* child1_output = fdopen(parent_from_child1_stdin_pipe[READ_END], "r");
        if (child1_output == NULL)
        {
            free_resources();
            error_exit("Could not open parent-child1 stdin pipe end for reading the result from the child!");
        }
        read_complex_numbers(child1_output, resultE, args.size);
        if (fclose(child1_output) != 0)
        {
            free_resources();
            error_exit("Could not close parent-child1 stdin pipe end after reading the result from the child!");
        }

        //Read second result from child 2's stdout.
        FILE* child2_output = fdopen(parent_from_child2_stdin_pipe[READ_END], "r");
        if (child2_output == NULL)
        {
            free_resources();
            error_exit("Could not open parent-child2 stdin pipe end for reading the result from the child!");
        }
        read_complex_numbers(child2_output, resultO, args.size);
        if (fclose(child2_output) != 0)
        {
            free_resources();
            error_exit("Could not close parent-child2 stdin pipe end after reading the result from the child!");
        }

        //Now we have both (half-)results (RE and RO). Calculate the total result.
        result = calc_FFT();

        //Print the result to stdout.
        print_complex_numbers(result, args.size, stdout);
    }
    else
    {
        //If input size is 1, simply print it.
        print_floats(args.numbers, args.size, stdout);
    }

    free_resources();
    exit(EXIT_SUCCESS);
}