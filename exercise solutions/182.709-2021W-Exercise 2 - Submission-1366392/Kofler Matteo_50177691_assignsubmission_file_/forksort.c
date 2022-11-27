/**
 * @file forksort.c
 * @author Matteo Kofler <e11904672@student.tuwien.ac.at>
 * @brief This program implements the forksort function by using pipes, fork and recursive execution of the program
 * @date 2021-12-01
 * 
 * This program implementa a reduced variation of the Unix-command expand,
 * which reads in several files (or from stdin if no file is given) and
 * replaces all tabulator characters with spaces. 
 * The amount of spaces which the tabulator character is replaced with is calculated
 * via a tabstop distance. 
 * Once the calculation is done, the processed text is either saved to a file 
 * or written to stdout.
 * 
 * For Compilation, the following was used:
 * gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mygrep.c 
 * To use makefile, type:
 * make all OR make forksort 
 * To "reset", type:
 * make clean all OR make clean forksort 
 * 
 * Doxygen help: https://embeddedinventor.com/guide-to-configure-doxygen-to-document-c-source-code-for-beginners/
 **/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h> 
#include <signal.h>
#include <sys/wait.h>

/** Holds the name of the program */
static char *programName;

/** Holds all lines that were given by input file/stdin */
static char **lines;

/** Holds all lines that were processed by the left child */
static char **linesLeft;

/** Holds all lines that were processed by the right child */
static char **linesRight;

/** enum containing which side a child is on (left or right) */
enum side {LEFT, RIGHT};

/** Number representing the amount of all lines from stdin */
static int lineCount;

/** Number representing the amount of lines processed by left child */
static int leftLineCount;

/** Number representing the amount of lines processed by right child */
static int rightLineCount;

/**
 * Error handling.
 * @brief In case of an error in the main method, this function is called.
 * @details
 * An error will be printed to stderr and the program stopped with EXIT_FAILURE
 * @param errMessage The error message to be printed
 * @param argv The name of the program
 * @return void, but program exits with EXIT_FAILURE
 */
static void errorAndExit(char *errMessage) {
    free(lines);
    free(linesRight);
    free(linesLeft);
    fprintf(stderr, "%s: %s \n", programName, errMessage);
    exit(EXIT_FAILURE);
}

/**
 * 
 * @brief Takes in input from a file and processes it into a child lines array by enum side
 * @param childSide, either LEFT or RIGHT, decides if the input is stored in linesLeft or linesRight
 * @param file, the file to be read from which is the output from the child via pipe
 * @return - no return.
 * @details 
 * This function is called by the parent process.
 * Input from file is processed into the linesLeft or linesRight array depending on the childSide enum
 * Uses linesLeft to save lines into a string array (if enum childSide is LEFT)
 * Uses linesRight to save lines into a string array (if enum childSide is RIGHT)
 * 
 */
static void readIntoSideArray(enum side childSide, FILE *file) {
    if (childSide != LEFT && childSide != RIGHT) {
        errorAndExit("Child is not left and not right");
    }
    int totalCharCount = 0;
    int lineCharCount = 0;
    /** used as index for the line number */
    int currentLine = 0;
    char c;
    /** 
     * if newline was last char, newLineLastChar is used for memory allocation of the new line
     * Not directly at the newline char because it is unknown if another line follows that
     * needs memory allocation
     *  
    */
    bool newLineLastChar = false;
    while ( (c = fgetc(file)) != EOF) {
        /** if last char was newline, allocate new memory for this new line */
        if (newLineLastChar) {
            lineCharCount = 0;
            currentLine++;
            if (childSide == LEFT) {
                linesLeft[currentLine] = (char*) malloc(1*sizeof(char));
            } else {
                linesRight[currentLine] = (char*) malloc(1*sizeof(char));
            }
        }
        newLineLastChar = false;
        if (c == '\n') {
            newLineLastChar = true;
        }
        /** save char as last position of array and allocate memory dynamically */
        totalCharCount++;
        lineCharCount++;
        if (childSide == LEFT) {
            linesLeft[currentLine] = (char*) realloc(linesLeft[currentLine], lineCharCount * sizeof(char));
            linesLeft[currentLine][lineCharCount-1] = c;
        } else {
            linesRight[currentLine] = (char*) realloc(linesRight[currentLine], lineCharCount * sizeof(char));
            linesRight[currentLine][lineCharCount-1] = c;
        }
    } 
}

/**
 * 
 * @brief Forks a child process process output from the recursively called program executions
 * @param childSide, either LEFT or RIGHT, decides if the input is stored in linesLeft or linesRight
 * @param fd_stdin file desciptors to write from parent to child
 * @param fd_stdout file descriptor containg stdout from child and where parent reads
 * @return - no return.
 * @details 
 * Important: only executes for either right or left side
 * Forks a child process that calls program recursively. 
 * Parent waits for child to finish than uses pipe to read the child's output
 * The output is then via helper function processed into linesLeft or linesRight
 * The lines of left or right are already sorted because child has run the whole program 
 * and the parent process of the child process has used this function and afterwards the merge function
 
 * Uses leftLineCount as a limit of how many lines are written into left pipe
 * Uses lineCount as a limit of how many lines are written into right pipe
 * 
 */
static void processChildren(enum side childSide, int fd_stdin[2], int fd_stdout[2]) {
    if (childSide != LEFT && childSide != RIGHT) {
        errorAndExit("Child is not left and not right");
    }
    /** generate a new process */
    pid_t pid = fork();
    int i;
    switch(pid) {
        /** in case of an error */
        case -1:
            errorAndExit("cannot fork!");
            break;
        /** child tasks */
        case 0:
            close(fd_stdin[1]);
            close(fd_stdout[0]);
            /** redirect the reading desciptor to stdin --> when program starts, input read from stdin */
            dup2(fd_stdin[0], STDIN_FILENO);
            close(fd_stdin[0]);
            /** redirect the writing desciptor to stdout --> parents reads stdout of child */
            dup2(fd_stdout[1], STDOUT_FILENO);
            close(fd_stdout[1]);
            /**
             * execute program again
             * Recursively because that program will execute again until max one line is in stdin 
             * (check in main function (lineCount))
             * Hint: argv[0] always programName -> use programName as first arg 
            */
            execlp(programName, programName, NULL);
            break;
        /** parent tasks */
        default:
            close(fd_stdin[0]);
            close(fd_stdout[1]);
            if (childSide == LEFT) {
                /** write half the lines to the child process */
                for (i = 0; i < leftLineCount; i++) {
                    if (lines[i] != NULL) {
                        write(fd_stdin[1], lines[i], strnlen(lines[i], 512));
                    } else {
                        errorAndExit("error: line is null");
                    }
                }
            } else if (childSide == RIGHT) { 
                /** write half the lines to the child process */
                for (i = leftLineCount; i < lineCount; i++) {
                    if (lines[i] != NULL) {
                        write(fd_stdin[1], lines[i], strnlen(lines[i], 512));
                    } else {
                        errorAndExit("error: line is null");
                    }
                }
            } 
            close(fd_stdin[1]);
            /** after writing to child, wait for child to finish (pdf page 29) */
            int status;
            pid_t thePid;
            while((thePid = wait(&status)) != pid){
                if(thePid != -1){
                    continue;
                }
                if(errno == EINTR){
                    continue;
                }
                errorAndExit("Cannot wait! \n");
            }
            if (WEXITSTATUS(status) != EXIT_SUCCESS) {
                errorAndExit("Child exit not successful");
            }
            /** child wrote to stdout, now read from stdout (file descriptors redirected) */
            FILE *file;
            if ((file = fdopen(fd_stdout[0], "r")) == NULL) {
                errorAndExit("Could not read from child");
            };
            /** get lines into a array */
            readIntoSideArray(childSide, file);
            /** cleanup */
            fclose(file);
            close(fd_stdout[0]);
            break;
    } 
}

/**
 * 
 * @brief merges the two arrays for left and right side in ascending order
 * @param - no params.
 * @return - no return.
 * @details 
 * This function merges the array from the left child with the right child.
 * The values from the left are iterated and compared to the right.
 * Important: The left array and right array are already internally sorted.
 * Uses rightLineCount to get the amount of lines of the right part
 * Uses leftLineCount to get the amount of lines of the left part
 * Uses linesLeft to get values of the left part
 * Uses linesRight to get values of the right part
 * 
 */
static void mergeChildren() {
    int i = 0;
    int j = 0;
    int rightChildrenPrinted = 0;
    /** Iterate through left array */
    for (i = 0; i < leftLineCount; i++) {
        while(j < rightLineCount) {
            /** 
             * if element from right array is smaller, print it
             * and also check next element(s) from the right. Otherwise break
             */
            if ( (strcmp(linesLeft[i], linesRight[j])) > 0) {
                rightChildrenPrinted++;
                fprintf(stdout, "%s", linesRight[j]);
                j++;
            } else {
                break;
            }
        }
        fprintf(stdout, "%s", linesLeft[i]);
    }
    /** Print the rest of elements in the right array */
    while(rightChildrenPrinted < rightLineCount) {
        fprintf(stdout, "%s", linesRight[rightChildrenPrinted]);
        rightChildrenPrinted++;
    }
}

/**
 * 
 * @brief Takes in input from stdin and processes it into lines array
 * @param - no params.
 * @return - no return.
 * @details 
 * This function is called in the beginning of the program execution. 
 * Input from stdin is processed into the lines array.
 * Uses lineCount to set the amount of lines in the input
 * Uses lines to save lines into a string array
 * 
 */
static void processInput(void) {
    lines = malloc(1*sizeof(*lines));
    int totalCharCount = 0;
    int lineCharCount = 0;
    /** index for the line in lines */
    lineCount = 1;
    char c;
    /** 
     * if newline was last char, newLineLastChar is used for memory allocation of the new line
     * Not directly at the newline char because it is unknown if another line follows that
     * needs memory allocation
     *  */
    bool newLineLastChar = false;
    while ( (c = fgetc(stdin)) != EOF) {
        if (newLineLastChar) {
            lineCharCount = 0;
            lineCount++;
            lines = realloc(lines, lineCount * sizeof(*lines));
            lines[lineCount-1] = (char*) malloc(1*sizeof(char));
        }
        newLineLastChar = false;
        if (c == '\n' ) {
            newLineLastChar = true;
        } 
        /** add char to array position and dynamically allocate memory */
        totalCharCount++;
        lineCharCount++;
        lines[lineCount-1] = (char *) realloc(lines[lineCount-1], lineCharCount * sizeof(char));
        lines[lineCount-1][lineCharCount-1] = c;
    } 
    /** if no chars were given as input, terminate with exit code */
    if (totalCharCount < 1) {
        errorAndExit("No chars given as input");
    }
    /** 
     * check if a newline is present at the end of the last line
     * if not, set a newline char at the end
     * This is important when array gets sorted and the element might not be at the end
     * Program would then always also read the next line into the same position as 
     * this line without the newline char
     */
    int lengthLastLine = strnlen(lines[lineCount-1], 512);
    if (lines[lineCount-1][lengthLastLine-1] != '\n') {
        lines[lineCount-1] = (char *) realloc(lines[lineCount-1], (lengthLastLine+1) * sizeof(char));
        lines[lineCount-1][lengthLastLine] = '\n';
    }
    /**
    for (i = 0; i < lineCount; i++) {
        printf("%s", lines[i]);
    }
    exit(EXIT_SUCCESS);
    */
}

/**
 * 
 * @brief Main function. Processes input from stdin into array, calls children for sorting and then merges and prints to stdout
 * @param argc the number of arguments given by user in command line
 * @param argv pointer to all the arguments given by user in command line 
 * @return - no return, program exits at the end.
 * @details 
 * Processes input from stdin into lines array via helper function processInput()
 * Then checks whether max one line is inputted -> if so, print to stdout and terminate
 * (if this happens parent also continues his work, if there is a parent)
 * Pipes and file desciptors, memory allocation are set up and init next
 * The recursive sorting for the left child is called next, then for the right
 * In that helper function fork and execlp is than called and the logic is handled
 * At the end, merge via helper function which prints to stdout * 
 * Uses rightLineCount to set the amount of lines of the right part
 * Uses leftLineCount to set the amount of lines of the left part
 * Uses linesLeft to allocate memory for array and free memory at the end
 * Uses linesRight to allocate memory for array and free memory at the end
 * Uses lines to free memory at the end
 * Uses programName to set the programName used by error messages and execlp in processChildren()
 * 
 */
int main(int argc, char *argv[]) {
    programName = argv[0];
    /** process lines from input */
    processInput();
    /** incase only one line is inputted, program writes it out and terminates */
    if (lineCount == 1) {
        if (lines[0] != NULL) {
            fprintf(stdout, "%s", lines[0]);
        } else {
            errorAndExit("Only one line inputted and line is null");
        }
    } else {
        /** create the arrays for the child arrays that are filled later */
        leftLineCount = lineCount / 2;
        rightLineCount = lineCount - leftLineCount;
		linesLeft = malloc((leftLineCount)*sizeof(char*));
		linesRight = malloc((rightLineCount)*sizeof(char*));

        /**
         * pipe before fork because file descriptors get copied over and inherited by child processes
         * two ends of file descriptor (pipe), [0] = reading, [1] = writing. 
         * close both on both child and parent asap.
         * Helpful for understanding: https://www.youtube.com/watch?v=Mqb2dVRe0uo
        */ 

        /** file descriptor used for writing from parent to left child */ 
        int fdLeftPipeStdin[2];
        /** file descriptor used for writing from left child to parent */ 
        int fdLeftPipeStdout[2];
        /** file descriptor used for writing from parent to right child */ 
        int fdRightPipeStdin[2];
        /** file descriptor used for writing from right child to parent */ 
        int fdRightPipeStdout[2];

        /** create pipes */
        if (pipe(fdLeftPipeStdin) == -1) {
            printf("Error while opening pipe");
            return -1;
        }
        if (pipe(fdLeftPipeStdout) == -1) {
            printf("Error while opening pipe");
            return -1;
        }
        if (pipe(fdRightPipeStdin) == -1) {
            printf("Error while opening pipe");
            return -1;
        }
        if (pipe(fdRightPipeStdout) == -1) {
            printf("Error while opening pipe");
            return -1;
        } 
        
        /** sort and merge left child by calling fork and execlp in function */
        processChildren(LEFT, fdLeftPipeStdin, fdLeftPipeStdout);
        /** sort and merge right child by calling fork and execlp in function */
        processChildren(RIGHT, fdRightPipeStdin, fdRightPipeStdout);
        /** merge left and right child and print solution in function */
        mergeChildren();
    }
    /** cleanup and exit */
    free(lines);
    free(linesLeft);
    free(linesRight);
    exit(EXIT_SUCCESS); 
}
