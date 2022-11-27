/**
 * @file forkFFT.c
 * @author Anton Martinovic 52004305
 * @brief A program to perform the Fast Foruier Transformation using fork exec and pipes
 * @date 2021-12-10
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

#define NUM_SIZE 25 // size of the first array which contains the input values
#define PI 3.1415926

char *programName;

/**
 * @brief struct which holds all pipes of the children
 * 
 */
typedef struct pipes
{
    int child1PipeRead[2];
    int child1PipeWrite[2];
    int child2PipeRead[2];
    int child2PipeWrite[2];
} pipe_children;

pipe_children program_pipes;

/**
 * @brief struct of a complex number
 * contains a float which is the real part 
 * and a second float with is the imaginary part
 * 
 */
typedef struct complex_num
{
    float re;
    float im;
} complex_number;

void parseToComp(char *lineTo, complex_number *complexNum);
void usage(void);
void closePipes(pipe_children *pipes);
complex_number mult(complex_number num1, complex_number num2);
complex_number subtraction(complex_number num1, complex_number num2);
complex_number addition(complex_number num1, complex_number num2);
int checkInput(char *line);
void wrongInput(void);

complex_number* algoResult(char *lineOne, char * lineTwo, FILE * evenIndicesRead, FILE *oddIndicesRead, int inputSize);

/**
 * @brief This function closes all pipes of the program
 * 
 * @param pipes struct, which contains all pipes
 */
void closePipes(pipe_children *pipes)
{
    close(pipes->child1PipeRead[0]);
    close(pipes->child1PipeRead[1]);
    close(pipes->child1PipeWrite[0]);
    close(pipes->child1PipeWrite[1]);
    close(pipes->child2PipeRead[0]);
    close(pipes->child2PipeRead[1]);
    close(pipes->child2PipeWrite[0]);
    close(pipes->child2PipeWrite[1]);
}
/**
 * @brief This function exits the program with EXIT_FAILURE and prints the correct Syntax of the input
 * 
 */
void usage(void)
{
    fprintf(stderr, "[%s] Synopsis: forkFFT\n", programName);
    exit(EXIT_FAILURE);
}
/**
 * @brief This function exits the program with EXIT_FAILURE and prints a message with an error number
 * 
 * @param msg message, that will be printed
 */
void error_exit(char *msg)
{
    fprintf(stderr, "[%s] %s %s\n", programName, msg, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief The main function performs the fft transformation. 
 * @param argc number of arguemnts passed. Should be 1
 * @param argv store the arguments that have been passed (should be contain only the program name)
 * @return the program exits with EXIT_SUCCESS in case of success or EXIT_FAILURE in case of failure
 */
int main(int argc, char **argv)
{

    programName = argv[0];

    if (argc > 1)
    {
        usage();
    }

    //two char* created to store the line. If only one line is contained, then this is printed and the process return with exit success

    char *lineOne = NULL;
    char *lineTwo = NULL;

    size_t lenOne;
    ssize_t readFirst;
    size_t lenTwo;
    ssize_t readSecond;

    int inputSize = 0;

    complex_number complex;

    if ((readFirst = getline(&lineOne, &lenOne, stdin)) == -1)
    {

        free(lineOne);
        fprintf(stderr, "[%s] No input!\n", programName);
        exit(EXIT_FAILURE);
       
    }

    if (checkInput(lineOne) == -1)
    {
        free(lineOne);
        wrongInput();
    }

    inputSize++;

    if ((readSecond = getline(&lineTwo, &lenTwo, stdin)) == -1)
    {
        parseToComp(lineOne, &complex);
        printf("%f %f\n", complex.re, complex.im);
        free(lineOne);
        free(lineTwo);
        exit(EXIT_SUCCESS);
    }
    //here the input is more or equal to 2, so we can fork and call the program recursevly
    if (checkInput(lineTwo) == -1)
    {
        free(lineOne);
        free(lineTwo);
        wrongInput();
    }
    inputSize++;

    // the pipes are being creates(2 per child) and fork to duplicate, filter the child and exec the program again with the child process

    if (pipe(program_pipes.child1PipeRead) == -1)
    {
        free(lineOne);
        free(lineTwo);
        error_exit("pipe child1 failed");
    }

    if (pipe(program_pipes.child1PipeWrite) == -1)
    {
        free(lineOne);
        free(lineTwo);
        error_exit("pipe child1 failed");
    }
    if (pipe(program_pipes.child2PipeRead) == -1)
    {
        free(lineOne);
        free(lineTwo);
        error_exit("pipe child2 failed");
    }
    if (pipe(program_pipes.child2PipeWrite) == -1)
    {
        free(lineOne);
        free(lineTwo);
        error_exit("pipe child2 failed");
    }

    pid_t idChild1;
    pid_t idChild2;


    idChild1 = fork();

    if (idChild1 == -1)
    {
        free(lineOne);
        free(lineTwo);
        error_exit("fork child1 failed");
    }
    
    else if (idChild1 == 0)
    {
        if (dup2(program_pipes.child1PipeRead[1], STDOUT_FILENO) == -1)
        {
            closePipes(&program_pipes);
            free(lineOne);
            free(lineTwo);
            error_exit("dup2 failed");
        }

        if (dup2(program_pipes.child1PipeWrite[0], STDIN_FILENO) == -1)
        {
            closePipes(&program_pipes);
            free(lineOne);
            free(lineTwo);
            error_exit("dup2 failed");
        }
        closePipes(&program_pipes);

        char *progam_args[] = {"./forkFFT", NULL};

        execv(progam_args[0], progam_args);

        //program should not come here, so we can print an error
        fprintf(stderr, "exec failed!\n");
        free(lineOne);
        free(lineTwo);
        exit(EXIT_FAILURE);
    }

    idChild2 = fork();

    if (idChild2 == -1)
    {
        free(lineOne);
        free(lineTwo);
        error_exit("fork child2 failed");
    }
    //pipe die schreibt, bekommt input(read end) von stdin
    // pipie die liest bekommt den input von stdout
    else if (idChild2 == 0)
    {
        if (dup2(program_pipes.child2PipeRead[1], STDOUT_FILENO) == -1)
        {
            closePipes(&program_pipes);
            free(lineOne);
            free(lineTwo);
            error_exit("dup2 failed");
        }

        if (dup2(program_pipes.child2PipeWrite[0], STDIN_FILENO) == -1)
        {
            closePipes(&program_pipes);
            free(lineOne);
            free(lineTwo);
            error_exit("dup2 failed");
        }
        closePipes(&program_pipes);

        //now program can be exec

        char *progam_args[] = {"./forkFFT", NULL};

        execv(progam_args[0], progam_args);

        fprintf(stderr, "exec failed!\n");
        free(lineOne);
        free(lineTwo);
        exit(EXIT_FAILURE);
    }

    close(program_pipes.child1PipeRead[1]);
    close(program_pipes.child1PipeWrite[0]);
    close(program_pipes.child2PipeRead[1]);
    close(program_pipes.child2PipeWrite[0]);

    //create file in which the results and the input will be saved

    FILE *evenIndicesWrite = NULL;
    FILE *oddIndicesWrite = NULL;
    FILE *evenIndicesRead = NULL;
    FILE *oddIndicesRead = NULL;

    evenIndicesWrite = fdopen(program_pipes.child1PipeWrite[1], "w");
    if (evenIndicesWrite == NULL)
    {
        closePipes(&program_pipes);
        fclose(evenIndicesRead);
        fclose(evenIndicesWrite);
        fclose(oddIndicesRead);
        fclose(oddIndicesWrite);
        free(lineOne);
        free(lineTwo);
        error_exit("fopen failed");
    }

    evenIndicesRead = fdopen(program_pipes.child1PipeRead[0], "r");

    if (evenIndicesRead == NULL)
    {
        closePipes(&program_pipes);

        fclose(evenIndicesRead);
        fclose(evenIndicesWrite);
        fclose(oddIndicesRead);
        fclose(oddIndicesWrite);
        free(lineOne);
        free(lineTwo);
        error_exit("fopen failed");
    }
    oddIndicesRead = fdopen(program_pipes.child2PipeRead[0], "r");

    if (oddIndicesRead == NULL)
    {
        closePipes(&program_pipes);

        fclose(evenIndicesRead);
        fclose(evenIndicesWrite);
        fclose(oddIndicesRead);
        fclose(oddIndicesWrite);
        free(lineOne);
        free(lineTwo);
        error_exit("fopen failed");
    }

    oddIndicesWrite = fdopen(program_pipes.child2PipeWrite[1], "w");
    if (oddIndicesWrite == NULL)
    {
        closePipes(&program_pipes);

        fclose(evenIndicesRead);
        fclose(evenIndicesWrite);
        fclose(oddIndicesRead);
        fclose(oddIndicesWrite);
        free(lineOne);
        free(lineTwo);
        error_exit("fopen failed");
    }

    //write the first two lines, which have been found, into the appropriate file
    fprintf(evenIndicesWrite, "%s", lineOne);
    fprintf(oddIndicesWrite, "%s", lineTwo);

    // complex_number *newComp = (complex_number *)malloc(NUM_SIZE * sizeof(complex_number));

    // parseToComp(lineOne, &newComp[0]);
    // parseToComp(lineTwo, &newComp[1]);

    int compLength = NUM_SIZE;
    
    
    //here the input is read from stdin and saved in the file of the children. Even indices written to one, odd to the other one

    while (getline(&lineOne, &lenOne, stdin) != -1)
    {

        if (checkInput(lineOne) == -1)
        {
            free(lineOne);
            free(lineTwo);
            // free(newComp);
            fclose(evenIndicesRead);
            fclose(evenIndicesWrite);
            fclose(oddIndicesRead);
            fclose(oddIndicesWrite);

            wrongInput();
        }

        if ((inputSize + 1) >= compLength)
        {
            compLength *= 2;
            // newComp = realloc(newComp, compLength * sizeof(newComp));
        }

        fprintf(evenIndicesWrite, "%s", lineOne);
        inputSize++;

        if (getline(&lineTwo, &lenTwo, stdin) != -1)
        {
            if (checkInput(lineTwo) == -1)
            {
                free(lineOne);
                free(lineTwo);
                // free(newComp);
                fclose(evenIndicesRead);
                fclose(evenIndicesWrite);
                fclose(oddIndicesRead);
                fclose(oddIndicesWrite);

                wrongInput();
            }
            fprintf(oddIndicesWrite, "%s", lineTwo);
        }
        else
        {
            free(lineOne);
            free(lineTwo);
            // free(newComp);
            fclose(evenIndicesRead);
            fclose(evenIndicesWrite);
            fclose(oddIndicesRead);
            fclose(oddIndicesWrite);
            fprintf(stderr, "[%s] Input has to be of 2^n\n", programName);
            exit(EXIT_FAILURE);
        }

        inputSize++;
        
    }

    

    //fd can be closed, because the data has been written into the files
    fclose(evenIndicesWrite);
    fclose(oddIndicesWrite);
    // free(newComp);

    int statusChild;

    //here we wait for the child processes to finish their job and check if the exited with success, if not, we cleanup and exit the program

    if (waitpid(idChild1, &statusChild, 0) == -1)
    {
        if (errno != EINTR)
        {
            free(lineOne);
            free(lineTwo);
            fclose(evenIndicesRead);
            fclose(oddIndicesRead);
            error_exit("Error at waitpid");
        }
    }

    if (WIFEXITED(statusChild))
    {
        if (WEXITSTATUS(statusChild) == EXIT_FAILURE)
        {
            free(lineOne);
            free(lineTwo);
            fclose(evenIndicesRead);
            fclose(oddIndicesRead);
            error_exit("Child process exited with EXIT_FAILURE");
        }
    }

    if (waitpid(idChild2, &statusChild, 0) == -1)
    {
        if (errno != EINTR)
        {
            free(lineOne);
            free(lineTwo);
            fclose(evenIndicesRead);
            fclose(oddIndicesRead);
            error_exit("Error at waitpid");
        }
    }
    if (WIFEXITED(statusChild))
    {
        if (WEXITSTATUS(statusChild) == EXIT_FAILURE)
        {
            free(lineOne);
            free(lineTwo);
            fclose(evenIndicesRead);
            fclose(oddIndicesRead);
            error_exit("Child process exited with EXIT_FAILURE");
        }
    }
    

    
    // array which contains the second part of the result, because of the formula
    complex_number secondHalf[inputSize / 2];

    //now here the childs have finished, and the data is in the childs files. Childs contains inputsize/2 of data, so we loop till inputsite/2 . 
    //we read from every file one line, and we calculate and print for the first half the results immediately to stdout, for the other half, we save it in the array which 
    //is created above
    for(int pos = 0; pos < inputSize/2; pos++)
    {

        complex_number evenChild;
        complex_number oddChild;
        complex_number result = {.im = 0, .re = 0};
        complex_number toPrint;

        if (getline(&lineOne, &lenOne, evenIndicesRead) == -1)
        {
            usage();
        }

        // if (checkInput(lineOne) == -1)
        // {
        //     fprintf(stderr, "Wrong input\n");
        //     exit(EXIT_FAILURE);
        // }
        parseToComp(lineOne, &evenChild);

        if (getline(&lineTwo, &lenTwo, oddIndicesRead) == -1)
        {
            usage();
        }
        // if (checkInput(lineTwo) == -1)
        // {
        //     fprintf(stderr, "Wrong input\n");
        //     exit(EXIT_FAILURE);
        // }
        parseToComp(lineTwo, &oddChild);

        result.re = (cos((-(2 * PI) / inputSize) * pos));
        result.im = (sin((-(2 * PI) / inputSize) * pos));

        toPrint = mult(result, oddChild);

        secondHalf[pos] = toPrint;

        secondHalf[pos] = subtraction(secondHalf[pos], evenChild);
        toPrint = addition(toPrint, evenChild);
        fprintf(stdout, "%f %f\n", toPrint.re, toPrint.im);
     
    }

    //here is the second part of the result printed
    for (int i = 0; i < inputSize / 2; i++)
    {
        fprintf(stdout, "%f %f\n", secondHalf[i].re, secondHalf[i].im);
    }

    free(lineOne);
    free(lineTwo);

    fclose(evenIndicesRead);
    fclose(oddIndicesRead);

    exit(EXIT_SUCCESS);
}




/**
 * @brief Performs an addition of two complex numbers
 * 
 * @param num1 
 * @param num2 
 * @return returns the result of the addition in form of a new complex_number
 */
complex_number addition(complex_number num1, complex_number num2)
{
    complex_number result = {.im = 0, .re = 0};

    result.re = num1.re + num2.re;
    result.im = num1.im + num2.im;

    return result;
}
/**
 * @brief This function subtracts num1 from num2
 * 
 * @param num1 subtrahend
 * @param num2 minuend
 * @return returns the result of the subtraction in form of a new complex_number
 */
complex_number subtraction(complex_number num1, complex_number num2)
{
    complex_number result = {.im = 0, .re = 0};

    result.re = num2.re - num1.re;
    result.im = num2.im - num1.im;

    return result;
}
/**
 * @brief Performs a multiplication of two complex numbers
 * 
 * @param num1 first complex number
 * @param num2 second complex number
 * @return returns the result of the multiplaction in form of a new complex_number 
 */
complex_number mult(complex_number num1, complex_number num2)
{
    complex_number result = {.im = 0, .re = 0};

    result.re = num1.re * num2.re - num1.im * num2.im;
    result.im = num1.re * num2.im + num1.im * num2.re;
    return result;
}

/**
 * @brief This function parses the argument, which are given by lineTo, to the complex_number struct which is passed. 
 * 
 * @param lineTo input of this line is passed to the complex_number struct
 * @param complexNum variable, in which the result is saved
 */
void parseToComp(char *lineTo, complex_number *complexNum)
{

    char *firstEnd;
    char *secondeEnd;

    complexNum->re = strtof(lineTo, &firstEnd);

    complexNum->im = strtof(firstEnd, &secondeEnd);
}
/**
 * @brief This function checks if the input by the user is in the correct format (Number [Number]). 
 * 
 * @param line the line that is checked
 * @return on success checkInput return 0, otherwise -1
 */
int checkInput(char *line)
{

    char *firstEnd;
    char *secondEnd;
    strtof(line, &firstEnd);
    strtof(firstEnd, &secondEnd);

    if (line == firstEnd)
    {
        return -1;
    }

    if (*secondEnd != '\n')
    {
        return -1;
    }
    return 0;
}
/**
 * @brief This function output an error message, which signals, that the input format was wrong. Further its exits the program.
 */

void wrongInput(void)
{
    fprintf(stderr, "[%s] Input wrong format!\nExpected: Number [Number]\n", programName);
    exit(EXIT_FAILURE);
}