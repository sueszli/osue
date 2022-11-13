/**
 * @file forkFFT.c 
 * @author Philipp Vanek (12022484)
 * @date 22.11.2021
 * 
 * @details Calculates a FFT for input given via stdin.
 * If input only has one number, writes number directly to stdout.
 * If input is an uneven count of numbers, exits with failue code.
 * Otherwise send numbers with even index to one child and uneven index to second child.
 * After reading complex numbers from child-pipe calculate the FFT and write result to stdout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <math.h>

/** global variable to save program path/name */
static const char *myprog;



/**
 * @brief Prints the programm path, an error message
 * @param errorMsg string message which should be printed
 */
static void outputError(char* errorMsg) {
    fprintf(stderr, "%s: %s \n", myprog, errorMsg);
}


/**
 * @brief converts a given string to a float number.
 * 
 * @param line string which should be converted to float.
 * @param number reference to location where number shall be saved.
 * @return negative value if an error occured.
 */
static int convertLineToFloat(char* line, float* number) {
    
    char* endOfNumberPointer;
    *number = strtof(line, &endOfNumberPointer);

    if (endOfNumberPointer == line) { // if no number was read
        outputError("Argument doesn't start with number!");
        return -1;
    }
    if (errno != 0) {
        outputError("No Conversion performed!");
        return -1;
    }
    if ((*endOfNumberPointer != '\n') && (*endOfNumberPointer != '\0')) {
        outputError("Argument doesn't end with number or linebreak!");
        return -1;
    }

    return 0;
}


/**
 * @brief reads lines from stdin and converts them to floats. 
 * 
 * @param realNumbers reference to array where input shall be saved.
 * @return negative value if an error occured. 
 */
static int readFromStdIn(float** realNumbers) {
    char * line = NULL;
    size_t len = 0;
    int numbersRead = 0;

    int floatArraySize = 2;
    
    *realNumbers = malloc(2*sizeof(float)); // can expect at least two values initially;
    if(*realNumbers == NULL) {
        outputError("Couldn't allocate memory!");
        exit(EXIT_FAILURE);
    }

    while (getline(&line, &len, stdin) != -1) {
        // strtol converts string to float
        float number = 0;
        if(convertLineToFloat(line, &number) < 0) {
            outputError("Invalid input!");
            free(*realNumbers);
            free(line);
            return -1;
        }

        // reallocate memory if array not big enough
        if(numbersRead == floatArraySize) {
            floatArraySize *= 2;
            float* newRealNumbers = realloc(*realNumbers, floatArraySize*2*sizeof(float));
            if(newRealNumbers == NULL) {
                free(line);
                free(*realNumbers);
                outputError("Couldn't reallocate memory!");
                return -1;
            }
            *realNumbers = newRealNumbers;
        }

        (*realNumbers)[numbersRead] = number;
        numbersRead++;
    }
    free(line);
    return numbersRead;
}


/**
 * @brief closes a pipe and prints an error message if an error occurs.
 * 
 * @param pipeFD pipe to be closed.
 * @return true if pipe was closed sucessfully.
 * @return false if an error occured.
 */
static bool closePipe(int pipeFD) {
    if( close(pipeFD) < 0 ) {
        outputError("Couldn't close pipe!");
        return false;
    }
    return true;
}



/**
 * @brief forks a child process and handles the closing and mapping of pipes.
 * 
 * @param pid of child which was created.
 * @param readFromChild pipe to read from child process.
 * @param writeToChild pipe to write to child process.
 * @return negative value if an error occured. 
 */
static int forkAChild(pid_t* pid, int* readFromChild, int* writeToChild) {
    
    fflush(stdout);

    // Create pipes
    if(pipe(readFromChild) < 0) {
        outputError("Couldn't create pipe!");
        return -1;
    }
    if(pipe(writeToChild) < 0) {
        closePipe(readFromChild[0]);
        closePipe(readFromChild[1]);
        outputError("Couldn't create pipe!");
        return -1;
    }
    

    // fork the child process
    *pid = fork();
    switch (*pid) {
        case -1:

            closePipe(readFromChild[0]);
            closePipe(readFromChild[1]);
            closePipe(writeToChild[0]);
            closePipe(writeToChild[1]);
            outputError("Cannot fork!");
            return -1;

        case 0: // child tasks
                        
            // close unused ends
            if ((closePipe(readFromChild[0]) && closePipe(writeToChild[1])) == false) {
                return -1;
            }


            // set pipe to stdin and stdout
            if((dup2(writeToChild[0], STDIN_FILENO) < 0) || (dup2(readFromChild[1], STDOUT_FILENO) < 0)) {
                closePipe(readFromChild[1]);
                closePipe(writeToChild[0]);
                outputError("Couldn't dup2!");
                return -1;
            }

            // close remaining ends
            if ((closePipe(readFromChild[1]) && closePipe(writeToChild[0])) == false) {
                return -1;
            }

            execlp("./forkFFT", "forkFFT", NULL);

            // should not reach here if exec worked correctly
            outputError("Cannot exec!"); 
            return -1;
            
        default: // parent task
            // close unused ends
            if ((closePipe(readFromChild[1]) && closePipe(writeToChild[0])) == false) {
                return -1;
            }
            break;
    }

    return 0;
}



/**
 * @brief Splits an array in two arrays, one containing values from even indices, the other from uneven indices.
 * 
 * @param arrayLength length of array which shall be split.
 * @param arrayToSplit array to split into two parts.
 * @param arrayEven array containing values from even indices.
 * @param arrayUneven array containing values from uneven indices.
 */
static void splitArrayByEvenAndUnevenIndex(int arrayLength, float* arrayToSplit, float* arrayEven, float* arrayUneven) {  
    for (int i = 0; i < arrayLength; i += 2) {
        arrayEven[i/2] = arrayToSplit[i];
    }
    for (int i = 1; i < arrayLength; i += 2) {
        arrayUneven[i/2] = arrayToSplit[i];
    }
}



/**
 * @brief converts a given string into a complex float number.
 * 
 * @param line string to be converted.
 * @param realNumber real-component of the complex number.
 * @param imgNumber imaginery-component of the complex number.
 * @return negative value if an error occured. 
 */
static int convertLineToComplexFloat(char* line, float* realNumber, float* imgNumber) {
    
    char* endOfNumberPointer;
    *realNumber = strtof(line, &endOfNumberPointer);

    
    if (endOfNumberPointer == line) { // if no number was read
        outputError("Argument doesn't start with number!");
        return -1;
    }
    if (errno != 0) {
        outputError("No Conversion performed!");
        return -1;
    }

    // look for ' ' after first number
    if(endOfNumberPointer[0] != ' ') {
        outputError("Argument doesn't have ' ' between numbers!");
        return -1;
    }

    line = endOfNumberPointer+1; // skip the ' '


    *imgNumber = strtof(line, &endOfNumberPointer);
    if (endOfNumberPointer == line) { // if no number was read
        outputError("Argument doesn't start with number!");
        return -1;
    }
    if (errno != 0) {
        outputError("No Conversion performed!");
        return -1;
    }

    // look for ' ' after first number
    if((endOfNumberPointer[0] != '*') || (endOfNumberPointer[1] != 'i')) {
        outputError("Argument doesn't have '*i' after second number!");
        return -1;
    }

    if ((endOfNumberPointer[2] != '\n') && (endOfNumberPointer[2] != '\0')){
        outputError("Argument doesn't end with number or linebreak!");
        return -1;
    }

    return 0;
}



/**
 * @brief reads multiple complex numbers from a child process.
 * 
 * @param numbersRead how many numbers the parent process(calling process) has read from stdin.
 * @param pipeEnd pipe id where input shall be read from.
 * @param childReal reference(array) where real-components of numbers read shall be saved.
 * @param childImg reference(array) where imaginery-components of numbers read shall be saved.
 * @return negative value if an error occured. 
 */
static int readFromChild(int numbersRead, int pipeEnd, float*  childReal, float* childImg) {
    char * line = NULL;
    size_t len = 0;
    
    FILE* readChildFile = fdopen(pipeEnd, "r");
    if(readChildFile == NULL) {
        outputError("Couldn't open pipe as file!");
        return -1;
    }

    int i = 0;
    while (getline(&line, &len, readChildFile) != -1) {
        
        float realNumber = 0;
        float imgNumber = 0;

        if(numbersRead == 2) {
            if(convertLineToFloat(line, &realNumber) < 0) {
                outputError("Couldn't convert input to numbers!");
                free(line);
                fclose(readChildFile);
                return -1;
            }
        }
        else {
            if(convertLineToComplexFloat(line, &realNumber, &imgNumber) < 0) {
                

                outputError("Couldn't convert input to numbers!");
                free(line);
                fclose(readChildFile);
                return -1;
            }
        }    
        childReal[i] = realNumber;
        childImg[i] = imgNumber;

        i++;
    }

    free(line);
    fclose(readChildFile);

    return 0;
}


/**
 * @details 
 * reads input numbers from stdin.
 * splits array of read numbers into even and uneven indices.
 * forks two child processes and handles read and write pipe for each child.
 * writes array of even indices to first child. 
 * writes array of uneven indices to second child. 
 * reads complex numbers from both children.
 * calculate FFT with complex numbers from child processes.
 * writes the calculated FFT to stdout.
 * waits for child processes to finish.
 */
int main(int argc, char *argv[]) {


    myprog = argv[0]; // Save the programm name globally

    if(argc > 1) {
        outputError("Correct usage: ./forkFFT");
        exit(EXIT_FAILURE);
    }

    // read stuff from stdIn 
    float* realNumbers;

    int numbersRead = 0;
    if((numbersRead = readFromStdIn(&realNumbers)) < 1) {
        outputError("Couldn't read numbers from stdin!");
        exit(EXIT_FAILURE);
    }
    
    // print to stdout and exit
    if(numbersRead == 1) {
        printf("%f\n", realNumbers[0]);
        free(realNumbers);
        exit(EXIT_SUCCESS);
    }
    else if((numbersRead % 2) == 1) {
        outputError("Uneven amout of number read!");
        free(realNumbers);
        exit(EXIT_FAILURE);
    }


    // split array in even and uneven parts
    float arrayEven[numbersRead/2];
    float arrayUneven[numbersRead/2];
    splitArrayByEvenAndUnevenIndex(numbersRead, realNumbers, arrayEven, arrayUneven);
    free(realNumbers);



    // prepare pipe and fork stuff
    pid_t childOneID;
    pid_t childTwoID;

    int readFromChildOne[2];
    int writeToChildOne[2];
    int readFromChildTwo[2];
    int writeToChildTwo[2];


    fflush(stderr);
    fflush(stdout);

    // fork the child processes
    if(forkAChild(&childOneID, readFromChildOne, writeToChildOne) < 0) {
        outputError("Couldn't fork child one!");
        exit(EXIT_FAILURE);
    }
    if(forkAChild(&childTwoID, readFromChildTwo, writeToChildTwo) < 0) {
        outputError("Couldn't fork child two!");
        closePipe(readFromChildOne[0]);
        closePipe(writeToChildOne[1]);
        exit(EXIT_FAILURE);
    }


    // Send stuff to child one
    for (int i = 0; i < numbersRead/2; i++) {
        char str[200];
        int x = snprintf(str, 200, "%f\n", arrayEven[i]);
        if((x < 0) || (x > 200)) {
            outputError("Couldn't convert float to string!");
            closePipe(readFromChildOne[0]);
            closePipe(writeToChildOne[1]);
            closePipe(readFromChildTwo[0]);
            closePipe(writeToChildTwo[1]);
            exit(EXIT_FAILURE);
        }

        if(write(writeToChildOne[1], str, strlen(str)) < 0) {
            outputError("Couldn't write to child one!");
            closePipe(readFromChildOne[0]);
            closePipe(writeToChildOne[1]);
            closePipe(readFromChildTwo[0]);
            closePipe(writeToChildTwo[1]);
            exit(EXIT_FAILURE);
        }
    }
    if(closePipe(writeToChildOne[1]) == false) {
            outputError("Couldn't close write to child one pipe!");
            closePipe(readFromChildOne[0]);
            closePipe(readFromChildTwo[0]);
            closePipe(writeToChildTwo[1]);
            exit(EXIT_FAILURE);
    }
    

    // Send stuff to child two
    for (int i = 0; i < numbersRead/2; i++) {
        
        char str[200];
        int x = snprintf(str, 200, "%f\n", arrayUneven[i]);
        if((x < 0) || (x > 200)) {
            outputError("Couldn't convert float to string!");
            closePipe(readFromChildOne[0]);
            closePipe(readFromChildTwo[0]);
            closePipe(writeToChildTwo[1]);
            exit(EXIT_FAILURE);
        }

        if(write(writeToChildTwo[1], str, strlen(str)) < 0) {
            outputError("Couldn't write to child two!");
            closePipe(readFromChildOne[0]);
            closePipe(readFromChildTwo[0]);
            closePipe(writeToChildTwo[1]);
            exit(EXIT_FAILURE);
        }
    }
    if(closePipe(writeToChildTwo[1]) == false) {
            outputError("Couldn't close write to child two pipe!");
            closePipe(readFromChildOne[0]);
            closePipe(readFromChildTwo[0]);
            exit(EXIT_FAILURE);
    }
    
    
    // read from child one
    float childOneReal[numbersRead/2];
    float childOneImg[numbersRead/2];

    if(readFromChild(numbersRead, readFromChildOne[0], childOneReal, childOneImg) < 0) {
        outputError("Couldn't read from child one!");
        closePipe(readFromChildTwo[0]);
        exit(EXIT_FAILURE);
    }


    // read from child two
    float childTwoReal[numbersRead/2];
    float childTwoImg[numbersRead/2];
    
    if(readFromChild(numbersRead, readFromChildTwo[0], childTwoReal, childTwoImg) < 0) {
        outputError("Couldn't read from child two!");
        exit(EXIT_FAILURE);
    }



    // calculate with the return values of children (FFT)
    float calculatedValuesReal[numbersRead];
    float calculatedValuesImg[numbersRead];
    
    for (int k = 0; k < numbersRead/2; k++) {

        //  (a + i · b)(c + i · d) = a·c − b·d + i·(a·d + b·c)
        double a = cos( ((-2)*M_PI*k) / numbersRead);
        double b = sin( ((-2)*M_PI*k) / numbersRead); 

        // R[k]
        calculatedValuesReal[k] = childOneReal[k] + (a*childTwoReal[k] - b*childTwoImg[k]);
        calculatedValuesImg[k]  = childOneImg[k]  + (a*childTwoImg[k]  + b*childTwoReal[k]);

        // R[k + n/2]
        calculatedValuesReal[k + numbersRead/2] = childOneReal[k] - (a*childTwoReal[k] - b*childTwoImg[k]);
        calculatedValuesImg[k + numbersRead/2]  = childOneImg[k]  - (a*childTwoImg[k]  + b*childTwoReal[k]);
    }

    

    
    //write calculated stuff to stdout
    for (int i = 0; i < numbersRead; i++) {
        fprintf(stdout, "%f %f*i\n", calculatedValuesReal[i], calculatedValuesImg[i]);
    }
    fflush(stdout);
    



    // wait for children
    int status;
    bool childError = false;
    // wait till every child finished
    while (wait(&status) != -1) {
        if(WEXITSTATUS(status) != EXIT_SUCCESS) {
            outputError("Child process had an error!");
            childError = true;
        }
    }
    if(childError) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}