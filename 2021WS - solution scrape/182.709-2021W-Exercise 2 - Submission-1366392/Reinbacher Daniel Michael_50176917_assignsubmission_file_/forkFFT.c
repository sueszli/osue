/**
 * @file forkFFT.c
 * @author Daniel Reinbacher (01614435)
 * @date 2021-12-08
 * 
 * @brief forkFFT module. A program that takes a list of floats as input and calculates the fourier transform in
 * a recursive manner
 * 
 * @details The program reads from stdin line by line a list of floats. The size of the input must be 2^n. Then it
 * recursevly creates two child processes and passes to each process half the input. One half with all even indices,
 * the other half with all odd ones. It does so until the last child processes only have a single value. Then the parent
 * read the outputs from the children, calculate new results with them an pass those results to their parent unti the first
 * process has all n results and prints them to stdin. The communication with the children is done via two pipes per child.
 * One for passing down the inputs and one for reading the childs results. The only global variable used is char *myprog for
 * storing the programs name.
 * 
 */

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "errorhandler.h"

#define PI 3.141592654

typedef struct {
    float real;
    float imaginary;
} complex_t;

char *myprog;

/**
 * @brief this function reads a complex number from the specified file and stores it
 * into the given pointer for the result.
 * @details The function takes two parameters, a pointer to a complex_t type variable,
 * in which the read number will be stored and a FILE pointer to the file from which the
 * number should be read. First the function reads a line from the file with getline. If
 * there is no line to read, it returns -1. Otherwise it uses strtof to read the first float.
 * If no number could be read, the program exits with status EXIT_FAILURE. If the first number
 * could be read it assignes it to the real variable or the given result and attempts to read a
 * second number. The second number doesnt have to be given. If, after attempting to read the 
 * second number, the endpointer points to something else then nullterminator, newline or '*i',
 * the program exits with status EXIT_FAILURE. Otherwise it assigns the second number to the 
 * imaginary variable of the given result. 
 * 
 * @param result the variable to write the number to
 * @param fp the file to read from
 * @return int -1 if failure, 0 otherwise
 */
int readNumber(complex_t *result, FILE *fp) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    if ((nread = getline(&line, &len, fp)) == -1) {
        return -1;
    }
    char *endp;
    result->real = strtof(line, &endp);
    if (endp == line) {
        error_exit("invalid input", "one or two floats must be given");
    }
    result->imaginary = strtof(endp, &endp);
    if (*endp != '\0' && *endp != '\n' && strcmp(endp, "*i\0") != 0 &&
        strcmp(endp, "*i\n") != 0) {
        error_exit("invalid input", "one or two floats must be given");
    }
    return 0;
}

/**
 * @brief closes a given filedescriptor with error handling
 * 
 * @param pipefd the filedescriptor to close
 */
void closePipe(int pipefd) {
    if (close(pipefd) == -1) {
        error_exit("close falied", strerror(errno));
    }
}

/**
 * @brief creates a new pipe
 * 
 * @param pipefd the array to store the filrdescriptors to
 */
void createPipe(int pipefd[2]) {
    if (pipe(pipefd) == -1) {
        error_exit("pipe failed", strerror(errno));
    }
}

/**
 * @brief redirects stdin and stdout to the pipes read and write end and
 * closes the unused ends
 * 
 * @param pipefd_1 the pipe to redirect stdin to
 * @param pipefd_2 the pipe to redirect stdout to
 */
void preparePipesForChild(int pipefd_1[2], int pipefd_2[2]) {
    closePipe(pipefd_1[1]);
    if (dup2(pipefd_1[0], STDIN_FILENO) == -1) {
        error_exit("dup2 falied", strerror(errno));
    }
    closePipe(pipefd_1[0]);
    closePipe(pipefd_2[0]);
    if (dup2(pipefd_2[1], STDOUT_FILENO) == -1) {
        error_exit("dup2 falied", strerror(errno));
    }
    closePipe(pipefd_2[1]);
}


/**
 * @brief multiplies two complex numbers
 * 
 * @param a the first factor
 * @param b the second factor
 * @return complex_t the result
 */
complex_t multiply(complex_t a, complex_t b) {
    complex_t result;
    result.real = a.real * b.real - a.imaginary * b.imaginary;
    result.imaginary = a.real * b.imaginary + a.imaginary * b.real;
    return result;
}

/**
 * @brief adds two complex numbers
 * 
 * @param a the first summand
 * @param b the second summand
 * @return complex_t the result
 */
complex_t add(complex_t a, complex_t b) {
    complex_t result;
    result.real = a.real + b.real;
    result.imaginary = a.imaginary + b.imaginary;
    return result;
}

/**
 * @brief subtracts one complex number from another
 * 
 * @param a the minuend
 * @param b the subtrahend
 * @return complex_t the result
 */
complex_t subtract(complex_t a, complex_t b) {
    complex_t result;
    result.real = a.real - b.real;
    result.imaginary = a.imaginary - b.imaginary;
    return result;
}

/**
 * @brief writes all input values with even indices to one child process and all
 * input values with odd indices to the other.
 * @details the function takes two pipes and two complex numbers. Those are the two
 * first input values that the current process has already read. The function writes
 * the first value to one pipe and the second to the other. Then, in a loop, it reads
 * from stdin and alternating writes the input values to either the first or the second
 * pipe. 
 * 
 * @param pipefd_1 the pipe to write the input values with even indices
 * @param pipefd_3 the pipe to write the input values with odd indices
 * @param first_number the first number read from stdin
 * @param second_number the second number read from stdin
 * @return int the total number of input values read and passed on
 */
int passInputsToChildren(int pipefd_1[2], int pipefd_3[2],
                         complex_t first_number, complex_t second_number) {
    dprintf(pipefd_1[1], "%f %f\n", first_number.real, first_number.imaginary);
    dprintf(pipefd_3[1], "%f %f\n", second_number.real,
            second_number.imaginary);
    int count = 2;
    complex_t next_number;
    while (readNumber(&next_number, stdin) != -1) {
        if (count % 2 == 0) {
            dprintf(pipefd_1[1], "%f %f*i\n", next_number.real,
                    next_number.imaginary);
        } else {
            dprintf(pipefd_3[1], "%f %f*i\n", next_number.real,
                    next_number.imaginary);
        }
        count++;
    }
    return count;
}

/**
 * @brief reads from the two child processes stdout line by line, then calculates
 * new values based on those inputs and writes them to stdout
 * @details the function takes as parameters two pipes from which the input values
 * for the calculation should be read, the count as number of values that should be calculated
 * and a pointer results to store the calculated values to. First two FILE pointers are opened
 * with the read ends of the given pipes. Then, in a loop with n/2 iterations, a value from both
 * pipes is read and with those values two new values are calculated and stored in results. If
 * there are less then n/2 input values in a pipe, the program exits with status EXIT_FAILURE.
 * 
 * @param pipefd_2 the pipe from which the even indices are read
 * @param pipefd_4 the pipe from which the odd indices are read
 * @param count the total number of values to calculate
 * @param results the array storing the calculated values
 */
void readResultsFromChildren(int pipefd_2[2], int pipefd_4[2], int count,
                             complex_t *results) {
    FILE *fp1, *fp2;
    if ((fp1 = fdopen(pipefd_2[0], "r")) == NULL) {
        error_exit("fdopen falied", strerror(errno));
    }
    if ((fp2 = fdopen(pipefd_4[0], "r")) == NULL) {
        error_exit("fdopen falied", strerror(errno));
    }
    complex_t re, ro;
    for (int i = 0; i < count / 2; i++) {
        if (readNumber(&re, fp1) != -1 && readNumber(&ro, fp2) != -1) {
            complex_t helper = {cos(-(2 * PI / count) * i),
                                sin(-(2 * PI / count) * i)};
            results[i] = add(re, multiply(helper, ro));
            results[i + count / 2] = subtract(re, multiply(helper, ro));
        } else {
            error_exit("invalid input", "input length must be even");
        }
    }
}

/**
 * @brief The program recursevly calculates the forier transform of a given input from stdin and
 * outputs the result to stdout.
 * @details The program first checks if it has more then one value in stdin. If it only has one
 * value, it outputs that value to stdout. Otherwise it forks two more processes and connects to
 * each of those processes with two pipes. One for writing output to the childs stdin and one for 
 * reading input from the childs stdout. Then the program reads line by line from stdin and passes
 * each read value two one of the child processes in an alternating manner. Afterwards it reads line
 * by line from each childs stdout and uses those values to compute new values via a given formula 
 * and prints those values to stdout. When one of the children terminates with an error, the program
 * terminates with an error aswell.
 * 
 * @param argc the argument counter
 * @param argv the argument vector
 * @return int EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
    myprog = argv[0];
    complex_t first_number, second_number;
    if (readNumber(&first_number, stdin) == -1) {
        exit(EXIT_SUCCESS);
    }
    if (readNumber(&second_number, stdin) == -1) {
        printf("%f %f*i\n", first_number.real, first_number.imaginary);
        exit(EXIT_SUCCESS);
    }
    int pipefd_1[2], pipefd_2[2];
    createPipe(pipefd_1);
    createPipe(pipefd_2);
    pid_t cid_1 = fork();
    if (cid_1 == -1) {
        error_exit("fork failed", strerror(errno));
    } else if (cid_1 == 0) {
        preparePipesForChild(pipefd_1, pipefd_2);
        execl("./forkFFT", "forkFFT", NULL);
        error_exit("fork failed", strerror(errno));
    } else {
        int pipefd_3[2], pipefd_4[2];
        createPipe(pipefd_3);
        createPipe(pipefd_4);
        pid_t cid_2 = fork();
        if (cid_2 == -1) {
            error_exit("fork failed", strerror(errno));
        } else if (cid_2 == 0) {
            preparePipesForChild(pipefd_3, pipefd_4);
            execl("./forkFFT", "forkFFT", NULL);
            error_exit("fork failed", strerror(errno));
        } else {
            closePipe(pipefd_1[0]);
            closePipe(pipefd_2[1]);
            closePipe(pipefd_3[0]);
            closePipe(pipefd_4[1]);
            int count = passInputsToChildren(pipefd_1, pipefd_3, first_number,
                                             second_number);
            if (count % 2 != 0) {
                error_exit("invalid input", "input length must be even");
            }
            closePipe(pipefd_1[1]);
            closePipe(pipefd_3[1]);
            complex_t results[count];
            readResultsFromChildren(pipefd_2, pipefd_4, count, results);
            for (int i = 0; i < count; i++) {
                printf("%f %f*i\n", results[i].real, results[i].imaginary);
            }
            closePipe(pipefd_2[0]);
            closePipe(pipefd_4[0]);
            int status_1, status_2;
            waitpid(cid_1, &status_1, 0);
            if (WEXITSTATUS(status_1) != EXIT_SUCCESS) {
                error_exit("waitpid failed", strerror(errno));
            }
            waitpid(cid_2, &status_2, 0);
            if (WEXITSTATUS(status_2) != EXIT_SUCCESS) {
                error_exit("waitpid failed", strerror(errno));
            }
        }
    }
    exit(EXIT_SUCCESS);
}