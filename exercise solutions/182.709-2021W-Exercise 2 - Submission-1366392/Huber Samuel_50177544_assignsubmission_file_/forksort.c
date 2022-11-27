/**
 * module name: forksort.c
 * @author      Huber Samuel 11905181
 * @brief       sorts multiple lines of input
 * @details     forksort.c uses recursive merge sort with the help of child processes,
 *              to sort inputs from stdin by splitting the input in halves, distributes the lines to recursively called
 *              child processes in which always two lines are compared according to general merge sort implementations
 * @date        01.12.2021
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>


#define READ 0
#define WRITE 1

// pipe file descriptors - inputing
int fd1_input[2];
int fd2_input[2];

// pipe file descriptors - receiving
int fd1_output[2];
int fd2_output[2];

// stream files for inputing
FILE *input_1;
FILE *input_2;

// stream files for receiving
FILE *output_1;
FILE *output_2;

char *prog; // name of the called program

static void usage(void);
static void errorHandling(char *msg);
static void freeResources(void);
static void sendToChildren(char **line);
static int nextLine(FILE* output, char **line);
static void compareLines(char **line1, char **line2);
static void waitForChild(int pid_child);
static void closePipe(int *fd);

/**
 * @brief prints out usage message
 * @details printed message advises the user, on how to call program properly \n program exits afterwards\n
 *          global variables used: 'prog'
 */
static void usage(void) {
    fprintf(stderr, " [%s] USAGE: %s \n", prog,prog);
    exit(EXIT_FAILURE);
}

/**
 * @brief informs user about occurred error
 * @details prints message of occurred error\n
 *          global variables used: 'prog'
 * @param msg: msg regarding occurred error
 */
static void errorHandling(char *msg){
    fprintf(stderr, "[%s] ERROR: %s\n",prog,msg);
    freeResources();
    exit(EXIT_FAILURE);
}


/**
 * @brief frees up used resources
 * @details file descriptors and IO streams are being freed up, before exiting\n
 *          global variables used:\n
 *          'fd1_input', 'fd2_input', 'fd1_output', 'fd2_output',\n
 *          'input_1', 'input_2', 'output_1', 'output_2'
 */
static void freeResources(void){
    if (fd1_input[WRITE] != -1) {
        close(fd1_input[WRITE]);
        fd1_input[WRITE] = -1;
    }
    if (fd2_input[WRITE] != -1) {
        close(fd2_input[WRITE]);
        fd2_input[WRITE] = -1;
    }

    if (fd1_output[READ] != -1) {
        close(fd1_output[READ]);
        fd1_input[READ] = -1;
    }
    if (fd2_output[READ] != -1) {
        close(fd2_output[READ]);
        fd2_output[READ] = -1;
    }


    if (input_1 != NULL) {
        fclose(input_1);
        input_1 = NULL;
    }
    if (input_2 != NULL) {
        fclose(input_2);
        input_2 = NULL;
    }
    if (output_1 != NULL) {
        fclose(output_1);
        output_1 = NULL;
    }
    if (output_2 != NULL) {
        fclose(output_2);
        output_2 = NULL;
    }
}

/**
 * @brief closes pipes with error handling
 * @details handles errors if occuring while closing fd\n
 * @param fd: file describot of which to close pipe
 */
static void closePipe(int *fd){
    if (*fd != -1) {
        if (close(*fd) == -1) {
            errorHandling("closing pipe failed");
        }
        *fd = -1;
    }
}

/**
 * @brief distributes input lines to children
 * @details input from stdin is distributed alternately between two child processes\n
 *          global variables used:\n
 *          'input_1', 'input_2'
 * @param line: char* with read lines from stdin
 */
static void sendToChildren(char **line){
    char *next = malloc(sizeof(char));
    if(next == NULL){
        errorHandling("allocating next in sendToChildren failed");
    }

    int isEOF = 0;

    int length = 1;

    int childToggle = 1;

    while(isEOF != 1){

        isEOF = nextLine(stdin, &next);
        if(isEOF == 1) {
            *(strchr(*line, '\n')) = '\0';
            if(childToggle == 1){
                fprintf(input_1,"%s", *line);
                fprintf(input_2,"%s", next);
            } else {
                fprintf(input_2,"%s", *line);
                fprintf(input_1,"%s", next);
            }
        } else {
            if(childToggle == 1){
                fprintf(input_1,"%s", *line);
                childToggle = 2;
            } else {
                fprintf(input_2,"%s", *line);
                childToggle = 1;
            }
            length = strlen(next) + 1;
            *line = realloc(*line, length);
            if(*line == NULL){
                errorHandling("reallocating *line failed");
            }
            strcpy(*line, next);
        }

    }
    free(next);
}

/**
 * @brief reads next line from given IO stream
 * @details stream will be read until '\n' or EOF is encountered, being the end of the line\n
 * @param stream: given IO stream from which to read
 * @param line: char* to which read line should be written
 * @return int value as boolean if EOF has been reached
 */
static int nextLine(FILE* stream, char **line){
    char *next = malloc(2);
    if(next == NULL){
        errorHandling("allocating next in nextLine failed");
    }
    int length = 0;

    int count = 0;

    int c  = fgetc(stream);
    int isEOF = 0;

    while(c != '\n'){

        if(c == EOF){
            isEOF = 1;
            break;
        }

        next[count] = c;
        count++;
        next = realloc(next, count+1);
        if(next == NULL){
            errorHandling("reallocating next failed");
        }

        c = fgetc(stream);
    }

    if(!isEOF){
        next[count] = '\n';
        count++;
        next = realloc(next, count+1);
    }

    next[count] = '\0';

    length = count+1;

    *line = realloc(*line, length);
    if(line == NULL){
        errorHandling("reallocating line in nextLine failed");
    }

    strcpy(*line, next);

    free(next);

    if(count == 0){
        isEOF = 1;
    }

    return isEOF;
}

/**
 * @brief compares the two given lines
 * @details gets the next lines of the two streams, compares them,\n
 *          prints the the smaller one (therefore in alphabetical order) to stdout,\n
 *          if EOF is reached on one of the inputs, \n
 *          the other lines will be printed until none are left \n
 *          global variables used:\n
 *          'output_1', 'output_2'
 * @param line1: char* to which read lines from output_1 should be written
 * @param line2: char* to which read lines from output_2 should be written
 */
static void compareLines(char **line1, char **line2){
    int isEOF_1 = nextLine(output_1, line1);
    int isEOF_2 = nextLine(output_2, line2);

    int running = 1;

    char *ptr;

    while(running){

        if((ptr = strchr(*line1, '\n')) != NULL){
            *ptr = '\0';
        }

        if((ptr = strchr(*line2, '\n')) != NULL){
            *ptr = '\0';
        }

        if(strcmp(*line1, *line2) < 0){
            if(!isEOF_1) {
                fprintf(stdout, "%s\n", *line1);
                isEOF_1 = nextLine(output_1, line1);
            } else {
                if(isEOF_2){
                    fprintf(stdout, "%s\n", *line1);
                    fprintf(stdout, "%s", *line2);
                } else {
                    fprintf(stdout, "%s\n", *line1);
                    fprintf(stdout, "%s\n", *line2);
                }
                running = 0;
            }
        } else {
            if(!isEOF_2) {
                fprintf(stdout, "%s\n", *line2);
                isEOF_2 = nextLine(output_2, line2);
            } else {
                if(isEOF_1){
                    fprintf(stdout, "%s\n", *line2);
                    fprintf(stdout, "%s", *line1);
                } else {
                    fprintf(stdout, "%s\n", *line2);
                    fprintf(stdout, "%s\n", *line1);
                }
                running = 0;
            }
        }
    }

    if(isEOF_1){
        while(!isEOF_2){
            isEOF_2 = nextLine(output_2, line2);

            if((ptr = strchr(*line2, '\n')) != NULL){
                *ptr = '\0';
            }
            if(isEOF_2){
                fprintf(stdout, "%s", *line2);
            } else {
                fprintf(stdout, "%s\n", *line2);
            }
        }
    } else {
        while(!isEOF_1){
            isEOF_1 = nextLine(output_1, line1);
            if((ptr = strchr(*line1, '\n')) != NULL){
                *ptr = '\0';
            }
            if(isEOF_1){
                fprintf(stdout, "%s", *line1);
            } else {
                fprintf(stdout, "%s\n", *line1);
            }
        }
    }
}

/**
 * @brief waits for child processes to end
 * @details if child is not properly ended, exit will be a failure\n
 * @param pid_child: pid of child to be waited for
 */
static void waitForChild(int pid_child){
    int status;
    pid_t pid;
    while((pid = waitpid(pid_child, &status, 0)) != pid_child){
        if(pid != -1){
            continue;
        }
        if (errno == EINTR) {
            continue;
        }
        errorHandling("waiting for child failed");
    }

    if (WEXITSTATUS(status) != EXIT_SUCCESS) {
        errorHandling("exit of child failed");
    }
}

/**
 * @brief main method of program
 * @details handles program call and all its arguments,\n
 *          reads input, checks if only one line was given,\n
 *          initiates file descriptors,\n
 *          handles pipes, creates children,\n
 *          calls comparison of lines,\n
 *          global variables used:\n
 *          'prog', 'fd1_input', 'fd2_input', 'fd1_output', 'fd2_output',\n
 *          'input_1', 'input_2', 'output_1', 'output_2'
 * @param argc: amount of given arguments
 * @param argv: list of given arguments
 * @return if program run successful or with errors
 */
int main(int argc, char *argv[]) {
    prog = argv[0];

    if(argc != 1){
        fprintf(stderr, "[%s] ERROR: No arguments allowed!\n", prog);
        usage();
    }

    // check if only 1 input line

    char *line = malloc(sizeof(char));
    if(line == NULL){
        errorHandling("allocating line in main failed");
    }

    if(nextLine(stdin, &line) == 1){
        fprintf(stdout, "%s", line);
        free(line);
        exit(EXIT_SUCCESS);
    }

    //initialise fd for pipes to -1
    memset(fd1_input, -1, sizeof(fd1_input));
    memset(fd2_input, -1, sizeof(fd2_input));

    // create sending pipes

    if (pipe(fd1_input) == -1) {
        errorHandling("creating pipe fd1_input failed");
    }
    if (pipe(fd2_input) == -1) {
        errorHandling("creating pipe fd2_input failed");
    }

    //create receiving pipes

    if (pipe(fd1_output) == -1) {
        errorHandling("creating pipe fd1_output failed");
    }
    if (pipe(fd2_output) == -1) {
        errorHandling("creating pipe fd2_output failed");
    }

    // create first child

    pid_t pid_1 = fork();

    if (pid_1 == -1) {
        errorHandling("forking of pid_1 failed");
    } else if (pid_1 == 0) {
        // child

        //closing pipes for c2
        closePipe(&fd2_input[READ]);
        closePipe(&fd2_output[WRITE]);        
        closePipe(&fd2_input[WRITE]);
        closePipe(&fd2_output[READ]);
        

        // redirect pipe to stdin
        closePipe(&fd1_input[WRITE]);
        if(dup2(fd1_input[READ], STDIN_FILENO) == -1){
            errorHandling("dup2 on fd1_input[READ] failed");
        }
        closePipe(&fd1_input[READ]);

        // redirect stdout to pipe
        closePipe(&fd1_output[READ]);
        if(dup2(fd1_output[WRITE], STDOUT_FILENO) == -1){
            errorHandling("dup2 on fd1_output[WRITE] failed");
        }
        closePipe(&fd1_output[WRITE]);

        execlp(prog, prog, NULL);
        errorHandling("execlp failed");
    }

    closePipe(&fd1_input[READ]);
    closePipe(&fd1_output[WRITE]);

    // create second child

    pid_t pid_2 = fork();
    if (pid_2 == -1) {
        errorHandling("forking of pid_2 failed");
    } else if (pid_2 == 0) {
        // child

        //closing pipes for c1
        closePipe(&fd1_input[READ]);
        closePipe(&fd1_output[WRITE]);
        closePipe(&fd1_input[WRITE]);
        closePipe(&fd1_output[READ]);

        // redirect pipe to stdin
        closePipe(&fd2_input[WRITE]);
        if(dup2(fd2_input[READ], STDIN_FILENO) == -1){
            errorHandling("dup2 on fd2_input[READ] failed");
        }
        closePipe(&fd2_input[READ]);

        // redirect stdout to pipe
        closePipe(&fd2_output[READ]);
        if(dup2(fd2_output[WRITE], STDOUT_FILENO) == -1){
            errorHandling("dup2 on fd2_output[WRITE] failed");
        }
        closePipe(&fd2_output[WRITE]);

        execlp(prog, prog, NULL);
        errorHandling("execlp failed");
    }

    closePipe(&fd2_input[READ]);
    closePipe(&fd2_output[WRITE]);

    // open stream io for writing to children
    input_1 = fdopen(fd1_input[WRITE], "w");
    input_2 = fdopen(fd2_input[WRITE], "w");

    // send input to children
    sendToChildren(&line);

    // close pipes for sending
    if (input_1 != NULL) {
        fclose(input_1);
        input_1 = NULL;
    }
    if (input_2 != NULL) {
        fclose(input_2);
        input_2 = NULL;
    }

    // open stream io for reading from children
    output_1 = fdopen(fd1_output[READ], "r");
    closePipe(&fd1_output[WRITE]);

    output_2 = fdopen(fd2_output[READ], "r");
    closePipe(&fd2_output[WRITE]);

    char* line1 = malloc(sizeof(char));
    if(line1 == NULL){
        errorHandling("allocating line1 in main failed");
    }
    char* line2 = malloc(sizeof(char));
    if(line2 == NULL){
        errorHandling("allocating line2 in main failed");
    }


    compareLines(&line1, &line2);

    // close pipes for receiving
    if (output_1 != NULL) {
        fclose(output_1);
        output_1 = NULL;
    }
    if (output_2 != NULL) {
        fclose(output_2);
        output_2 = NULL;
    }

    waitForChild(pid_1);
    waitForChild(pid_2);

    free(line);
    free(line1);
    free(line2);

    freeResources();
    
    exit(EXIT_SUCCESS);

}