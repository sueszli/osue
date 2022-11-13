/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * @module  forkFFT                                                                     *
 *                                                                                      *
 * @author  Richard Alscher - 11775285                                                  *
 *                                                                                      *
 * @brief   Calculates the FFT using the butterfly algorithm                            *
 *                                                                                      *
 * @details This program calculates the FFT using the butterfly algorithm by forking    *
 *          new child processes and passing the parameters on to them if at least       *
 *          two values have been submitted. If only one value is submitted or left      *
 *          in the child process, then the program returns it by printing to stdout.    *
 *          A child will pass the value on to the parent like this, the last parent     *
 *          will then print the result to the screen.                                   *
 *                                                                                      *
 * @date    18.11.2021                                                                  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <wait.h>
#include <math.h>

typedef struct {
    float realPart;
    float imaginaryPart;
} complexNumber;

/* always in a pipe[], pipe[0] is for read and pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1

#define DELIMITER " "

char  *programName;
char  *buffer;
complexNumber *complexNumbersFromLeftChild;
complexNumber *complexNumbersFromRightChild;

pid_t pidLeftChild;
pid_t pidRightChild;

/**
 * @brief   Exits the program on failure
 *
 * @details Frees resources and exits the program.
 *          Is called on program failure or in case of an error.
 *
 * @param   void
 * @return  void
 */
void exitFailure( void ) {
    free(buffer);
    free(complexNumbersFromLeftChild);
    free(complexNumbersFromRightChild);
    exit(EXIT_FAILURE);
}

/**
 * @brief   Exits the program on success
 *
 * @details Frees resources and exits the program.
 *          Is called on successful execution of the program.
 *
 * @param   void
 * @return  void
 */
void exitSuccess( void ) {
    free(buffer);
    free(complexNumbersFromLeftChild);
    free(complexNumbersFromRightChild);
    exit(EXIT_SUCCESS);
}

/**
 * @brief   Prints the usage message and exits the program
 *
 * @details Prints the usage message and exits the program
 *
 * @param   void
 * @return  void
 */
void usage( void ) {
    fprintf( stderr, "Usage: %s\n", programName );
    exitFailure();
}

/**
 * @brief   Calculates the real part of the multiplication that is part of the FFT
 *
 * @details Calculates the real part of the multiplication that is part of the FFT.
 *          Uses the formula:
 *          ( a + i * b ) * ( c + i * d ) = a * c − b * d + i * ( a * d + b * c )
 *          and calculates the part ( a * c − b * d )
 *
 * @param  k           int            is the index used in the formula: For all k | 0 <= k < n/2
 * @param  n           int            is the number of elements that are in the result set
 * @param  rightChild  complexNumber  the elements from R_O, so the odd index elements
 * @return             float          the result of the multiplication of the complex numbers
 */
float calculateRealPartFFT( int k, int n, complexNumber rightChild ) {

    return ( ( cos(  ( - ( 2*M_PI ) / n ) * k ) ) * rightChild.realPart ) - ( ( sin(  ( - ( 2*M_PI ) / n ) * k ) ) * rightChild.imaginaryPart );
}

/**
 * @brief   Calculates the imaginary part of the multiplication that is part of the FFT
 *
 * @details Calculates the imaginary part of the multiplication that is part of the FFT.
 *          Uses the formula:
 *          ( a + i * b ) * ( c + i * d ) = a * c − b * d + i * ( a * d + b * c )
 *          and calculates the part i * ( a * d + b * c ), without returning the i
 *
 * @param  k           int            is the index used in the formula: For all k | 0 <= k < n/2
 * @param  n           int            is the number of elements that are in the result set
 * @param  rightChild  complexNumber  the elements from R_O, so the odd index elements
 * @return             float          the result of the multiplication of the complex numbers
 */
float calculateImaginaryPartFFT( int k, int n, complexNumber rightChild ) {

    return ( ( cos(  ( - ( 2*M_PI ) / n ) * k ) ) * rightChild.imaginaryPart ) + ( rightChild.realPart * ( sin(  ( - ( 2*M_PI ) / n ) * k ) ) );
}

/**
 * @brief   Parses an input string into a complex number (struct)
 *
 * @details Parses an input string into a complex number with a given delimiter.
 *          Splits the string into two halfs and assigns them to the corresponding struct parameter.
 *
 * @param  string    char*          The string that is parsed
 * @param  delimiter char*          The delimiter that is used for splitting the string
 * @return           complexNumber  The complex number that has been parsed from the input string
 */
complexNumber parseComplexNumber(char *string, char *delimiter) {

    char *token;
    char *endPointer;
    complexNumber cn = {0, 0};
    token = strtok( string, delimiter);

    if (token == NULL) {
        exitFailure();
    }
    cn.realPart = strtof(token, &endPointer);
    if (endPointer == NULL) {
        exitFailure();
    }
    token = strtok(NULL, delimiter);
    if (token != NULL) {

        cn.imaginaryPart = strtof(token, &endPointer);
        if (endPointer == NULL) {
            exitFailure();
        }

    }

    return cn;
}


/**
 * @brief   The main function
 *
 * @details Performs the main functionality of the program, spawns child processes via fork,
 *          branches off until only one result remains and then performs the calculation.
 *          Exits when one branch has an odd number of inputs.
 *
 * @param  argc  int
 * @param  argv  int
 * @return       int  EXIT_SUCCESS on success or EXIT_FAILURE on error.
 */
int main( int argc, char *argv[] ) {

    programName = argv[0];

    FILE *input = stdin;
    buffer = NULL;
    size_t bufferSize = 0;

    if (argc > 1) {
        usage();
    }

    float failSafeBuffer2[2];
    int failSafeCounter = 0;

    while ( failSafeCounter != 2 && getline(&buffer, &bufferSize, input) != -1 ) {

        if (buffer[0] != '\n') {
            failSafeBuffer2[failSafeCounter] = strtof(buffer, NULL);
            //fprintf( stderr, "[DEBUG][PID %d]: past failSafeBuffer [ %d ]! FSB content: %f\n", getpid(), failSafeCounter, failSafeBuffer2[failSafeCounter]);
            failSafeCounter++;
        }

    }

    if (failSafeCounter == 0) {
        exitSuccess();
    } else if (failSafeCounter == 1) {
        fprintf(stdout, "%f\n", failSafeBuffer2[0]);
        exitSuccess();
    }

    int parentToChildLeftPipeFD[2], childToParentLeftPipeFD[2], parentToChildRightPipeFD[2], childToParentRightPipeFD[2];
    //fprintf(stderr, "[DEBUG][PID %d]: now piping\n", getpid());
    if (pipe(parentToChildLeftPipeFD) == -1 || pipe(childToParentLeftPipeFD) == -1 ||
        pipe(parentToChildRightPipeFD) == -1 || pipe(childToParentRightPipeFD) == -1) {
        fprintf(stderr, "[%s] --- [Error]: Could not open Pipe!\n", programName);
        exitFailure();
    }

    //fprintf(stderr, "[DEBUG][PID %d]: now forking left child\n", getpid());
    pidLeftChild = fork();

    // Check if left child could be forked
    if (pidLeftChild == -1) {
        fprintf(stderr, "[%s] --- [Error]: Cannot fork left child!\n", programName);
        exitFailure();
    } else if (pidLeftChild == 0) {
        // Forking of left child successful

        //fprintf(stderr, "[DEBUG][PID %d]: left child pid:  [%d]\n", getpid(), getpid());

        // close pipes for other child
        close(parentToChildRightPipeFD[READ_FD]); close(parentToChildRightPipeFD[WRITE_FD]);
        close(childToParentRightPipeFD[READ_FD]); close(childToParentRightPipeFD[WRITE_FD]);

        // close unused ends of pipes
        close(parentToChildLeftPipeFD[WRITE_FD]);
        close(childToParentLeftPipeFD[READ_FD]);

        // redirect the input and output to the respective ends of the pipes
        dup2(parentToChildLeftPipeFD[READ_FD], STDIN_FILENO);
        dup2(childToParentLeftPipeFD[WRITE_FD], STDOUT_FILENO);

        close(parentToChildLeftPipeFD[READ_FD]);
        close(childToParentLeftPipeFD[WRITE_FD]);

        // reset child to run from beginning
        execlp(programName, programName, NULL);

        // If this line is reached, something went wrong!
        fprintf(stderr, "[%s] --- [Error]: Child Process terminated ungracefully!\n", programName);
        exitFailure();
    } else {

        //fprintf(stderr, "[DEBUG][PID %d]: now forking right child\n", getpid());
        pidRightChild = fork();

        if (pidRightChild == -1) {
            fprintf(stderr, "[%s] --- [Error]: Cannot fork right child!\n", programName);
            exitFailure();
        } else if (pidRightChild == 0) {
            // Forking of right child successful

            //fprintf(stderr, "[DEBUG][PID %d]: right child pid: [%d]\n", getpid(), getpid());

            // Close pipes for other child
            close(parentToChildLeftPipeFD[READ_FD]); close(parentToChildLeftPipeFD[WRITE_FD]);
            close(childToParentLeftPipeFD[READ_FD]); close(childToParentLeftPipeFD[WRITE_FD]);

            // Close unused ends of pipes
            close(parentToChildRightPipeFD[WRITE_FD]);
            close(childToParentRightPipeFD[READ_FD]);

            // Redirect the input and output to the respective ends of the pipes
            dup2(parentToChildRightPipeFD[READ_FD], STDIN_FILENO);
            dup2(childToParentRightPipeFD[WRITE_FD], STDOUT_FILENO);

            close(parentToChildRightPipeFD[READ_FD]);
            close(childToParentRightPipeFD[WRITE_FD]);

            // Reset child to run from beginning
            execlp(programName, programName, NULL);

            // If this line is reached, something went wrong!
            fprintf(stderr, "[%s] --- [Error]: Child Process terminated ungracefully!\n", programName);
            exitFailure();
        }

        //fprintf(stderr, "[DEBUG][PID %d]: BACK IN THE PARENT!\n", getpid());

        close(childToParentLeftPipeFD[WRITE_FD]);
        close(childToParentRightPipeFD[WRITE_FD]);
        close(parentToChildLeftPipeFD[READ_FD]);
        close(parentToChildRightPipeFD[READ_FD]);

        //fprintf(stderr, "[DEBUG][PID %d]: WRITING TO CHILDREN!\n", getpid());
        //fprintf(stderr, "[DEBUG][PID %d]: FSB[0]: %f!\n", getpid(), failSafeBuffer2[0]);
        //fprintf(stderr, "[DEBUG][PID %d]: FSB[1]: %f!\n", getpid(), failSafeBuffer2[1]);
        dprintf(parentToChildLeftPipeFD[WRITE_FD], "%f\n", failSafeBuffer2[0]);
        dprintf(parentToChildRightPipeFD[WRITE_FD], "%f\n", failSafeBuffer2[1]);

        int count = 2;
        int writeToLeft = 1;

        //fprintf(stderr, "[DEBUG][PID %d]: WAITING FOR GETLINE IN PARENT!\n", getpid());
        while ( getline(&buffer, &bufferSize, input) != -1 ) {
            if (writeToLeft == 1) {
                writeToLeft = 0;
                dprintf(parentToChildLeftPipeFD[WRITE_FD], "%s", buffer);
            } else {
                writeToLeft = 1;
                dprintf(parentToChildRightPipeFD[WRITE_FD], "%s", buffer);
            }
            count++;
            //fprintf(stderr, "[DEBUG][PID %d]: GETLINE IN PARENT! Count: %d - Buffer: %s", getpid(), count, buffer);
        }
        //fprintf(stderr, "[DEBUG][PID %d]: Count: %d\n", getpid(), count);

        close(parentToChildLeftPipeFD[WRITE_FD]);
        close(parentToChildRightPipeFD[WRITE_FD]);

        if ( count % 2 != 0 ) {
            //fprintf(stderr, "[DEBUG][PID %d]: -[EXIT]-, ODD NUMBER OF INPUTS DETECTED!!!\n", getpid());
            exitFailure();
            close(childToParentLeftPipeFD[READ_FD]);
            close(childToParentRightPipeFD[READ_FD]);
        }

        int status;
        //fprintf(stderr, "[DEBUG][PID %d]: at wait(status) left?? child!\n", getpid());
        wait(&status);

        // Check the first child process if it failed, if not, continue to wait
        if (status == -1 /*|| WEXITSTATUS(status) != EXIT_SUCCESS*/) {
            exitFailure();
        } else {
            //fprintf(stderr, "[DEBUG][PID %d]: at wait(status) right?? child!\n", getpid());
            wait(&status);
        }

        // Check the second child process if it failed, if not, continue to wait
        if (status == -1 /*|| WEXITSTATUS(status) != EXIT_SUCCESS*/) {
            exitFailure();
        } else {
            // Both children ran successfully, process data now

            //fprintf(stderr, "[DEBUG][PID %d]: READING FROM LEFT CHILD!\n", getpid());
            // read from left child
            int currentCapacityLeftChildRead = 2;
            int leftChildReadCounter = 0;
            complexNumbersFromLeftChild = malloc(sizeof (complexNumber) * currentCapacityLeftChildRead);
            FILE* pipeStreamLeftChild = fdopen(childToParentLeftPipeFD[READ_FD], "r");
            while( getline(&buffer, &bufferSize, pipeStreamLeftChild) != -1 ) {

                if (currentCapacityLeftChildRead <= leftChildReadCounter) {
                    currentCapacityLeftChildRead *= 2;
                    complexNumbersFromLeftChild = realloc(complexNumbersFromLeftChild, sizeof (complexNumber) * currentCapacityLeftChildRead);
                    if (complexNumbersFromLeftChild == NULL) {
                        exitFailure();
                    }
                }

                complexNumbersFromLeftChild[leftChildReadCounter] = parseComplexNumber(buffer, DELIMITER);
                leftChildReadCounter++;
            }
            fclose(pipeStreamLeftChild);
            close(childToParentLeftPipeFD[READ_FD]);

            //fprintf(stderr, "[DEBUG][PID %d]: counted results of left child: %d\n", getpid(), leftChildReadCounter);
            /*
            for (int i = 0; i < leftChildReadCounter; i++) {
                fprintf(stderr, "[DEBUG][PID %d]: left child received %d: %f %f*i\n", getpid(), i, complexNumbersFromLeftChild[i].realPart, complexNumbersFromLeftChild[i].imaginaryPart);
            }
            */

            //fprintf(stderr, "[DEBUG][PID %d]: READING FROM RIGHT CHILD!\n", getpid());
            // read from right child
            int currentCapacityRightChildRead = 2;
            int rightChildReadCounter = 0;
            complexNumbersFromRightChild = malloc(sizeof (complexNumber) * currentCapacityRightChildRead);
            FILE* pipeStreamRightChild = fdopen(childToParentRightPipeFD[READ_FD], "r");
            while( getline(&buffer, &bufferSize, pipeStreamRightChild) != -1 ) {

                if (currentCapacityRightChildRead <= rightChildReadCounter) {
                    currentCapacityRightChildRead *= 2;
                    complexNumbersFromRightChild = realloc(complexNumbersFromRightChild, sizeof (complexNumber) * currentCapacityRightChildRead);
                    if (complexNumbersFromRightChild == NULL) {
                        exitFailure();
                    }
                }

                complexNumbersFromRightChild[rightChildReadCounter] = parseComplexNumber(buffer, DELIMITER);
                rightChildReadCounter++;
            }
            fclose(pipeStreamRightChild);
            close(childToParentRightPipeFD[READ_FD]);

            //fprintf(stderr, "[DEBUG][PID %d]: counted results of right child: %d\n", getpid(), rightChildReadCounter);
            /*
            for (int i = 0; i < rightChildReadCounter; i++) {
                fprintf(stderr, "[DEBUG][PID %d]: right child received %d: %f %f*i\n", getpid(), i, complexNumbersFromRightChild[i].realPart, complexNumbersFromRightChild[i].imaginaryPart);
            }
            */

            // if both children sent data, let's go
            if (leftChildReadCounter > 0 && rightChildReadCounter > 0 && leftChildReadCounter == rightChildReadCounter) {
                //fprintf(stderr, "[DEBUG][PID %d]: left and right counter > 1 and equal!\n", getpid());
            } else {
                //fprintf(stderr, "[DEBUG][PID %d]: left and right counter either < 1 or unequal!\n", getpid());
            }

            //fprintf(stderr, "[DEBUG][PID %d]: Process data now!\n", getpid());

            int fftCounter = leftChildReadCounter + rightChildReadCounter;
            complexNumber *fftCalc = malloc(sizeof (complexNumber) * fftCounter);
            for (int count = 0; count < fftCounter / 2; count++) {

                fftCalc[count].realPart = complexNumbersFromLeftChild[count].realPart + calculateRealPartFFT(count, fftCounter, complexNumbersFromRightChild[count]);
                fftCalc[count].imaginaryPart = complexNumbersFromLeftChild[count].imaginaryPart + calculateImaginaryPartFFT(count, fftCounter, complexNumbersFromRightChild[count]);

                fftCalc[count + fftCounter / 2].realPart = complexNumbersFromLeftChild[count].realPart - calculateRealPartFFT(count, fftCounter, complexNumbersFromRightChild[count]);
                fftCalc[count + fftCounter / 2].imaginaryPart = complexNumbersFromLeftChild[count].imaginaryPart - calculateImaginaryPartFFT(count, fftCounter, complexNumbersFromRightChild[count]);

            }

            // send to parent/output
            for (int i = 0; i < fftCounter; i++) {
                fprintf(stdout, "%f %f*i\n", fftCalc[i].realPart, fftCalc[i].imaginaryPart);
            }

            // free after sending
            free(fftCalc);

            exitSuccess();

        }




    }

    exitSuccess();

}
