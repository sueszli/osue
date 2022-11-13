/**
 * @file forksort.c
 * @author Dorian Dragaj 11702371
 * @date 03.12.2021
 * @brief OSUE Exercise 2 forksort
 * @details The program sorts lines from stdin alphabetically.
 * If only one line is read it returns immediately. Otherwise two process children will be forked.
 * The parent sends the child its stdin to its two children alternately.
 * If the children are finnished the parent reads the stdout of the children via a pipe and merges them
 * alphabetically.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

//Global variable
char *myprog; /** The program name.*/

//Help methods

/**
 * Closes two filedescriptors
 * @brief Method to close two filedescriptors
 * @param pipe1 the first filedescriptor to be closed.
 * @param pipe2 the second filedescriptor to be closed.
 */
static void cleanupInOrOutParentPipes(int pipe1, int pipe2);

/**
 * Closes the filedescripors of a pipe if the given fd array is non null
 * @brief This method closes fds of eachs given pipe by calling cleanUpInOrOutParentPipes
 * @param pipe1 the first pipe to be closed.
 * @param pipe2 the second pipe to be closed.
 * @param pipe3 the thirs pipe to be closed.
 * @param pipe4 the fourth pipe to be closed.
 */
static void cleanupPipes(int *pipe1, int *pipe2, int *pipe3, int *pipe4);

/**
 * Initializes a given pipe and exits programm if a failure occured
 * @brief This method initializes a pipe and inserts the pipe's fds values in the pipeFds array
 * on error the method exits the programm and clean the given ressources accordingly  
 * @param pipeFds File descripors of the initialized pipe
 * @param line1 on error line1 will be freed
 * @param line2 on error line2 will be freed
 * @param pipe1 on error pipe1 will be closed
 * @param pipe2 on error pipe2 will be closed
 * @param pipe3 on error pipe3 will be closed
 */
static void initPipe(int *pipeFds, char *line1, char *line2, int *pipe1, int *pipe2, int *pipe3);

/**
 * An error handler to cleanup all given resources and exit the program with EXIT_FAILURE afterwards
 * @brief This method frees and closes all given parameters and exits the programm with EXIT_FAILURE
 * @param pipeFds File descripors of the initialized pipe
 * @param line1 line1 will be freed
 * @param line2 line2 will be freed
 * @param pipe1 pipe1 will be closed
 * @param pipe2 pipe2 will be closed
 * @param pipe3 pipe3 will be closed
 * @param pipe4 pipe4 will be closed
 */
static void handle_error(char *line1, char *line2, int *pipe1, int *pipe2, int *pipe3,  int *pipe4);

/**
 * Calls dup2 for each fd inputFd for the stdin redirection and the outputFd for stdout and handles an error if occured
 * @brief This method calls dup2 for each file descriptor respectively
 * inputFd will be redirected to the stdin and outputFd to the stdout, in order to read/write from/to the children.
 * If an error occure -1 will be returned otherwise 0
 * @param inputFd File descriptor to be redirected to stdin
 * @param outputFd File descriptor to be redirected to stdout 
 */
static int dup2ChildFds(int inputFd, int outputFd);

/**
 * Calls waitpid and waits for child process, and frees defined resources and exits on error
 * @brief This method calls waitpid and blocks till specified child process returns.
 * On error program will exit with status EXIT_FAILURE and defined ressources will be freed/closed.
 * If the child process terminated with EXIT_FAILURE, parent process shall also terminate with exit failure
 * @param child Child process to wait for
 * @param line1 line1 will be freed
 * @param line2 line2 will be freed
 * @param pipe1 pipe1 will be closed
 * @param pipe2 pipe2 will be closed
 */
static void waitForChild(pid_t child, char *line1, char *line2, int pipeFd1, int pipeFd2);

/**
 * The entrypoint of forksort. 
 * @brief The main entrypoint of this programm.
 * @details This method forks 2 children recursively and stops forking if the childs input is only 1 line long.
 * Here also the parent inputs are passed alternately via a write pipe to the children stdin. 
 * And the children stdouts (via read pipe) are merged by the parent alphabetically. 
 * @param argc The argument counter.
 * @param arv The argument vector.
 * @return If method successeeded EXIT_SUCCESS is returned, otherwise EXIT_FAILURE.
 */
int main(int argc, char *argv[]) {
    myprog = argv[0];

    // Check arguments
    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", myprog);
        exit(EXIT_FAILURE);
    }

    char *line1 = NULL;
    size_t len1 = 0;
    ssize_t read1 = 0;

    char *line2 = NULL;
    size_t len2 = 0;
    ssize_t read2 = 0;

    // Create child A and B
    pid_t child_a, child_b;

    //Check if stdin contains multiple lines
    if((read1 = getline(&line1, &len1, stdin)) == -1 && feof(stdin) == 0) {
        fprintf(stderr, "%s: getLine failed: %s\n", myprog, strerror(errno));
        free(line1);
        exit(EXIT_FAILURE);
    }

    if(read1 == -1) {
        free(line1);
        exit(EXIT_SUCCESS);
    }

    if((read2 = getline(&line2, &len2, stdin)) == -1 && feof(stdin) == 0) {
        fprintf(stderr, "%s: getLine failed: %s\n", myprog, strerror(errno));
        free(line1);
        free(line2);
        exit(EXIT_FAILURE);
    }

    if(read2 == -1) {
        if (line1[strlen(line1) - 1] == '\n') {
            fprintf(stdout, "%s", line1);
        } else {
            fprintf(stdout, "%s\n", line1);
        }
        free(line1);
        free(line2);
        exit(EXIT_SUCCESS);
    }

    // Init Input Pipes
    int inputPipeA[2];
    int outputPipeA[2];
    int inputPipeB[2];
    int outputPipeB[2];

    initPipe(inputPipeA, line1, line2, NULL, NULL, NULL);
    initPipe(outputPipeA, line1, line2, inputPipeA, NULL, NULL);
    initPipe(inputPipeB, line1, line2, inputPipeA, outputPipeA, NULL);
    initPipe(outputPipeB, line1, line2, inputPipeA, outputPipeA, inputPipeA);

    // Create child A
    child_a = fork();

    if (child_a == -1) {
        fprintf(stderr, "%s: Failed to fork child A: %s\n", myprog,
                strerror(errno));
        handle_error(line1, line2, inputPipeA, inputPipeB, outputPipeA, outputPipeB);
    }

    if (child_a == 0) {
        // Setup child A
        // Configure input pipe for child A
        if (dup2ChildFds(inputPipeA[0], outputPipeA[1]) == -1) {
            handle_error(line1, line2, inputPipeA, inputPipeB, outputPipeA, outputPipeB);
        }
        cleanupPipes(inputPipeA, inputPipeB, outputPipeA, outputPipeB);
        execlp(myprog, myprog, NULL);
        fprintf(stderr, "%s: Failed to execlp for child A: %s\n", myprog,
                strerror(errno));
        handle_error(line1, line2, inputPipeA, inputPipeB, outputPipeA, outputPipeB);
    } else {
        // Create child B
        child_b = fork();

        if (child_b == -1) {
            fprintf(stderr, "%s: Failed to fork child B: %s\n", myprog,
                    strerror(errno));
            handle_error(line1, line2, inputPipeA, inputPipeB, outputPipeA, outputPipeB);
        }

        if (child_b == 0) {
            // Setup child B
            // Configure input pipe for child B
            if (dup2ChildFds(inputPipeB[0], outputPipeB[1]) == -1) {
                handle_error(line1, line2, inputPipeA, inputPipeB, outputPipeA, outputPipeB);
            }
            cleanupPipes(inputPipeA, inputPipeB, outputPipeA, outputPipeB);
            execlp(myprog, myprog, NULL);
            fprintf(stderr, "%s: Failed to execlp for child B: %s\n", myprog,
                    strerror(errno));
            handle_error(line1, line2, inputPipeA, inputPipeB, outputPipeA, outputPipeB);
        }
    }
    // Close unused children pipes for parent
    cleanupInOrOutParentPipes(inputPipeA[0], inputPipeB[0]);
    cleanupInOrOutParentPipes(outputPipeA[1], outputPipeB[1]);
    // Open input write pipe of child A
    FILE *writeChildA = fdopen(inputPipeA[1], "w");
    if (writeChildA == NULL) {
        fprintf(stderr,
                "%s: Failed to open input write stream of child A: %s\n",
                myprog, strerror(errno));
        cleanupInOrOutParentPipes(inputPipeA[1], inputPipeB[1]);
        cleanupInOrOutParentPipes(outputPipeA[0], outputPipeB[0]);
        free(line1);
        free(line2);
        exit(EXIT_FAILURE);
    }

    // Open input write pipe of child B
    FILE *writeChildB = fdopen(inputPipeB[1], "w");
    if (writeChildB == NULL) {
        fprintf(stderr,
                "%s: Failed to open input write stream of child B: %s\n",
                myprog, strerror(errno));
        cleanupInOrOutParentPipes(outputPipeA[0], outputPipeB[0]);
        close(inputPipeB[1]);
        fclose(writeChildA);
        free(line1);
        free(line2);
        exit(EXIT_FAILURE);
    }

    // Forward input to children
    FILE *childrenIn[2] = {writeChildA, writeChildB};
    int posSwitcher = 0;

    fprintf(childrenIn[posSwitcher], "%s", line1);
    fprintf(childrenIn[!posSwitcher], "%s", line2);

    // Alternate between children stdins
    while(getline(&line1, &len1, stdin) != -1) {
        fprintf(childrenIn[posSwitcher], "%s", line1);
        posSwitcher = !posSwitcher; 
    }

    // Close children input write pipes
    fclose(childrenIn[0]); // Also closes FD
    fclose(childrenIn[1]); // Also closes FD

    // wait for children to terminate
    waitForChild(child_a, line1, line2, outputPipeA[0], outputPipeB[0]);
    waitForChild(child_b, line1, line2, outputPipeA[0], outputPipeB[0]);

    // Open children output read1 pipes
    // Open output read1 pipe of child A
    FILE *readChildA = fdopen(outputPipeA[0], "r");
    if (readChildA == NULL) {
        fprintf(stderr,
                "%s: Failed to open output read1 stream of child A: %s\n",
                myprog, strerror(errno));
        cleanupInOrOutParentPipes(outputPipeA[0], outputPipeB[0]);
        free(line1);
        free(line2);
        exit(EXIT_FAILURE);
    }

    // Open output read1 pipe of child B
    FILE *readChildB = fdopen(outputPipeB[0], "r");
    if (readChildB == NULL) {
        fprintf(stderr,
                "%s: Failed to open output read1 stream of child B: %s\n",
                myprog, strerror(errno));
        close(outputPipeB[0]);
        fclose(readChildA);
        free(line1);
        free(line2);
        exit(EXIT_FAILURE);
    }

    //Merge children
    read1 = getline(&line1, &len1, readChildA);
    read2 = getline(&line2, &len2, readChildB);

    do {
        if(read1 == -1) {
            //This means Child A is empty (reached EOF)
            //Print remaining lines of Child B and exit loop
            while (read2 != -1) {
                fprintf(stdout, "%s", line2);
                read2 = getline(&line2, &len2, readChildB);
            }
            break;
        } else if(read2 == -1) {
            //This means Child B is empty (reached EOF)
            while (read1 != -1) {
                fprintf(stdout, "%s", line1);
                read1 = getline(&line1, &len1, readChildA);
            }
            //Print remaining lines of Child A and exit loop
            break;
        } else {
            //We need a special compare that compares lines without newlines and if the line consists only of one newLine it should not be removed
            if (strcmp(line1, line2) < 0) {
                fprintf(stdout, "%s", line1);
                read1 = getline(&line1, &len1, readChildA);
            } else {
                fprintf(stdout, "%s", line2);
                read2 = getline(&line2, &len2, readChildB);
            }
        }
    } while(true);

    // Cleanup remaining resources
    free(line1);
    free(line2);

    if((read1 == -1) && (feof(readChildA) == 0)) {
        fprintf(stderr, "%s: getLine failed: %s\n", myprog, strerror(errno));
        fclose(readChildA);
        fclose(readChildB);
        exit(EXIT_FAILURE);
    }

    if((read2 == -1) && (feof(readChildB) == 0)) {
        fprintf(stderr, "%s: getLine failed: %s\n", myprog, strerror(errno));
        fclose(readChildB);
        exit(EXIT_FAILURE);
    }

    fclose(readChildA);
    fclose(readChildB);
    exit(EXIT_SUCCESS);
}

static void cleanupInOrOutParentPipes(int pipe1, int pipe2) {
    close(pipe1);
    close(pipe2);
}

static void cleanupPipes(int *pipe1, int *pipe2, int *pipe3,
                         int *pipe4) {
    if (pipe1 != NULL) {
        cleanupInOrOutParentPipes(pipe1[0], pipe1[1]);
    }
    if (pipe2 != NULL) {
        cleanupInOrOutParentPipes(pipe2[0], pipe2[1]);
    }
    if (pipe3 != NULL) {
        cleanupInOrOutParentPipes(pipe3[0], pipe3[1]);
    }
    if (pipe4 != NULL) {
        cleanupInOrOutParentPipes(pipe4[0], pipe4[1]);
    }
}

static void handle_error(char *line1, char *line2, int *pipe1, int *pipe2, int *pipe3,  int *pipe4) {
    free(line1);
    free(line2);
    cleanupPipes(pipe1, pipe2, pipe3, pipe4);
    exit(EXIT_FAILURE);
}

static void initPipe(int *pipeFds, char *line1, char *line2, int *pipe1, int *pipe2, int *pipe3) {
    //If pipe fails fd of pipe are not open
    if (pipe(pipeFds) == -1) {
        fprintf(stderr, "%s: Input pipe of child failed to initialize: %s\n",
                myprog, strerror(errno));
        handle_error(line1, line2, pipe1, pipe2, pipe3, NULL);
    }
}

static int dup2ChildFds(int inputFd, int outputFd) {
    if ((dup2(inputFd, STDIN_FILENO) == -1) || (dup2(outputFd, STDOUT_FILENO) == -1)) {
        fprintf(stderr, "%s: Failed to redirect stdin of child B: %s\n", myprog, strerror(errno));
        return -1;
    }
    return 0;
}

static void waitForChild(pid_t child, char *line1, char *line2, int pipeFd1, int pipeFd2) {
    int status = 0;

    if ((waitpid(child, &status, 0) == -1) || (WEXITSTATUS(status) == EXIT_FAILURE)) {
        cleanupInOrOutParentPipes(pipeFd1, pipeFd2);
        handle_error(line1, line2, NULL, NULL, NULL, NULL);
    }
}