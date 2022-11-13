/**
 * @file forksort.c
 * 
 * @author Filip Markovic 12024750
 * 
 * @brief This program sorts an input of lines recursively and prints the solution out.
 * 
 * @details First of all we save the lines to an array, then we open different child processes to which we redirect the lines.
 * These lines get piped to the child processes as stdin. These child processes run recursively until the line only constists
 * of one character, in which case we print the character. Stdout is also piped as an output pipe and the results get merged
 * in the parent class via the sorting algorithm. if every child is finished the result is printed to stdout.
 * 
 * @date 2021-12-10
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * @brief represents a child process, with it's own input and output pipe
 * 
 * @date 2021-12-10
 */
typedef struct {
    pid_t pid;
    int inputPipe[2];
    int outputPipe[2];
} child;

char const *SYNOPSIS = "forksort";
char const *PROGRAM = "./forksort";

/**
 * @brief prints an error which occured because of calling the program in a wrong way and terminates
 * 
 * @param error extra error message to be printed
 * 
 * @date 2021-12-10
 */
void usage(char *error){
    fprintf(stderr, "[%s]\nERROR: %s\n", SYNOPSIS, error);
    exit(EXIT_FAILURE);
}

/**
 * @brief prints an error and in difference to usage(...), also prints strerror(errno), and doesn't terminate
 * 
 * @param error extra error message to be printed
 * 
 * @date 2021-12-10
 */
void error(char *error){
    fprintf(stderr, "[%s]\nERROR: %s \n%s\n", SYNOPSIS, error, strerror(errno));
}

/**
 * @brief frees the lines from the memory space
 * 
 * @param lines pointer to the lines to be freed
 * @param lineAmount number of lines
 * 
 * @date 2021-12-10
 */
void freeLines(char ***lines, int *lineAmount){
    for (int i = 0; i <= *lineAmount; i++){
        free((*lines)[i]);
    }
    free(*lines);
}

/**
 * @brief reads the lines and saves them into an array.
 * 
 * @details reads line per line from stdin, terminates on EOF. If there is only one line read overall it gets printed to stdout.
 * 
 * @param lines pointer to the lines, where the read lines should be saved
 * @param lineAmount number of lines
 * 
 * @date 2021-12-10
 */
void readLines(char ***lines, int *lineAmount);

/**
 * @brief initializes the child 
 * 
 * @details initializes the child and its pipes. Then we fork and for child processes redirect its pipes to stdin and stdout.
 * if that is done we can recursively execute the program again. This is where the recursion happens.
 * 
 * @param child chilld structure which shall be initilialized
 * @return int return -1 on error and 0 on success
 * 
 * @date 2021-12-10
 */
int initChild(child *child);

/**
 * @brief initializes the child processes pipes via pipe(...)
 * 
 * @param child child process of which its pipes shall be initialized
 * @return int return -1 on error and 0 on success
 * 
 * @date 2021-12-10
 */
int initPipes(child *child);

/**
 * @brief redirect the pipes that the inputPipe is linked with stdin, and the outputPipe with stdout. Closes unused ends.
 * 
 * @param child child which pipes shall be redirected
 * @return int return -1 on error and 0 on success\
 * 
 * @date 2021-12-10
 */
int redirectPipes(child *child);

/**
 * @brief closes the param pipe via close(...)
 * 
 * @param pipe pipe to be closed
 * @return int return -1 on error and 0 on success
 * 
 * @date 2021-12-10
 */
int closePipe(int *pipe);

/**
 * @brief redirects the input to the children processes via fprintf
 * 
 * @details first we open a FILE for the inputPipes of both child processes to write into. The first half of the lines gets
 * printed to the first child, the second to the second child. After that we close the files again and are finished writing to
 * the children.
 * 
 * @param lines lines to be written to the children
 * @param child1 child to which the first half shall be written
 * @param child2 child to which the second half shall be written
 * @param lineAmount number of lines
 * @return int return -1 on error and 0 on success
 * 
 * @date 2021-12-10
 */
int inputToChildren(char ***lines, child *child1, child *child2, int *lineAmount);

/**
 * @brief reads the stdout output of the child processes and merges what was read alphabetically.
 * 
 * @details opens FILE pointers of the child processes output pipe and getline(...) for them to read their output, which is printed to stdout.
 * To sort the output we compare the strings after reading a line for each and so for the program the output is recursively sorted.
 * 
 * @param child1 child to read the output from (first half of lines were inserted here)
 * @param child2 child to read the output from (second half of lines were inserted here)
 * @return int return -1 on error and 0 on success
 * 
 * @date 2021-12-10
 */
int outputToParent(child *child1, child *child2);

/**
 * @brief waits for the child processes to finish via waitpid. If an error occurs the whole program exits.
 * 
 * @param child1 first child process we wait for
 * @param child2 second child process we wait for
 * 
 * @date 2021-12-10
 */
void waitForChildren(child *child1, child *child2);

/**
 * @brief The main program which runs the previously described functions to sort an input of lines.
 * 
 * @details First we read the lines and save them into an array. After that we create two child processes which are piped according to redirectPipes(...).
 * Now the parent and child processes can communicate via their pipes and redirect their output and input. If we write something from a child process to 
 * stdout, it is read from the parent process via stdin. After we have written everything to the child processes we check their output and sort it recursively
 * according to their alphabetical order (strcmp(...)). The last child processes print to the second last and so forth... . We wait until all the child processes 
 * are finished after that and then we can terminate the program succesfully.
 * 
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @return int return -1 on error and 0 on success
 * 
 * @date 2021-12-10
 */
int main(int argc, char const *argv[])
{
    if (argc > 1){
        usage("no arguments allowed");
    }

    int lineAmount = 0;
    char **lines = malloc(sizeof(int));

    readLines(&lines, &lineAmount);

    child child1, child2;

    if (initChild(&child1) == -1){
        fprintf(stderr, "initializing child1\n");
        freeLines(&lines, &lineAmount);
        exit(EXIT_FAILURE);
    }
    if (initChild(&child2) == -1){
        fprintf(stderr, "initializing child1\n");
        freeLines(&lines, &lineAmount);
        exit(EXIT_FAILURE);
    }

    if (inputToChildren(&lines, &child1, &child2, &lineAmount) == -1){
        fprintf(stderr, "forwarding input to children\n");
        freeLines(&lines, &lineAmount);
        exit(EXIT_FAILURE);
    }

    if (outputToParent(&child1, &child2) == -1){
        fprintf(stderr, "forwarding output to parent\n");
        freeLines(&lines, &lineAmount);
        exit(EXIT_FAILURE);
    }

    freeLines(&lines, &lineAmount);

    waitForChildren(&child1, &child2);

    exit(EXIT_SUCCESS);
}

void readLines(char ***lines, int *lineAmount){
    int count = 0;
    size_t lineSize = 0;
    size_t n = 0;
    int g = 0;

    while ((g = getline((&(*lines)[count]), &n, stdin)) != -1){
        lineSize += n;
        *lines = realloc(*lines, lineSize);
        count++;
    }
    if (count <= 1){
        if (count == 1){
            fprintf(stdout, "%s", (*lines)[0]);
        }
        freeLines(lines, &count);
        exit(EXIT_SUCCESS);
    }
    *lineAmount = count;
}

int initChild(child *child){
    if (initPipes(child) == -1){
        fprintf(stderr, "initializing pipes for child\n");
        return -1;
    }
    child->pid = fork();
    if (child->pid == -1){
        error("forking child");
        return -1;
    }

    if (child->pid == 0){
        redirectPipes(child);
        if (execlp(PROGRAM, PROGRAM, NULL) == -1){
            error("running execlp for child");
            return -1;
        }
    }

    if (closePipe(&child->inputPipe[0]) == -1){
        fprintf(stderr, "closing inputPipe[0]\n");
        return -1;
    }
    if (closePipe(&child->outputPipe[1]) == -1){
        fprintf(stderr, "closing outputPipe[1]\n");
        return -1;
    }
    return 0;
}

int initPipes(child *child){
    if (pipe(child->inputPipe) == -1){
        error("opening input pipe of child");
        return -1;
    }
    if (pipe(child->outputPipe) == -1){
        error("opening output pipe of child");
        return -1;
    }
    return 0;
}

int redirectPipes(child *child){
    if (dup2(child->inputPipe[0], STDIN_FILENO) == -1){
        error("redirecting child.inputPipe[0]");
        return -1;
    }
    if (closePipe(&child->inputPipe[1]) == -1){
        fprintf(stderr, "closing child.inputPie[1]\n");
        return -1;
    }
    if (dup2(child->outputPipe[1], STDOUT_FILENO) == -1){
        error("redirecting child.outputPipe[1]");
        return -1;
    }
    if (closePipe(&child->outputPipe[0]) == -1){
        fprintf(stderr, "closing child.outputPipe[0]\n");
        return -1;
    }
    if (closePipe(&child->inputPipe[0]) == -1){
        fprintf(stderr, "closing child.inputPie[0]\n");
        return -1;
    }
    if (closePipe(&child->outputPipe[1]) == -1){
        fprintf(stderr, "closing child.outputPipe[1]\n");
        return -1;
    }
    return 0;
}

int closePipe(int *pipe){
    if (close(*pipe) == -1){
        error("closing pipe");
        return -1;
    }
    return 0;
}

int inputToChildren(char ***lines, child *child1, child *child2, int *lineAmount){
    FILE *fileChild1 = fdopen(child1->inputPipe[1], "w");
    FILE *fileChild2 = fdopen(child2->inputPipe[1], "w");

    if (fileChild1 == NULL){
        error("fdopen, opening fileChild1");
        return -1;
    }
    if (fileChild2 == NULL){
        error("fdopen, opening fileChild2");
        return -1;
    }

    int middle = (*lineAmount) / 2;

    for (int i = 0; i < middle; i++){
        fprintf(fileChild1, "%s",  (*lines)[i]);
    }
    for (int i = middle; i < *lineAmount; i++){
        fprintf(fileChild2, "%s",  (*lines)[i]);
    }

    if (fclose(fileChild1) == -1){
        error("closing fileChild1");
        return -1;
    }
    if (fclose(fileChild2) == -1){
        error("closing fileChild2");
        return -1;
    }

    return 0;
}

int outputToParent(child *child1, child *child2){
    FILE *fileChild1 = fdopen(child1->outputPipe[0], "r");
    FILE *fileChild2 = fdopen(child2->outputPipe[0], "r");

    char *line1, *line2;
    size_t n1 = 0, n2 = 0;

    int g1 = getline(&line1, &n1, fileChild1);
    int g2 = getline(&line2, &n2, fileChild2);
    while(g1 != -1 && g2 != -1){
        if (strcmp(line1, line2) > 0){
            fprintf(stdout, "%s", line2);
            g2 = getline(&line2, &n2, fileChild2);
        } else {
            fprintf(stdout, "%s", line1);
            g1 = getline(&line1, &n1, fileChild1);
        }
    }
    while (g1 != -1){
        fprintf(stdout, "%s", line1);
        g1 = getline(&line1, &n1, fileChild1);
    }
    while (g2 != -1){
        fprintf(stdout, "%s", line2);
        g2 = getline(&line2, &n2, fileChild2);
    }

    free(line1);
    free(line2);

    if (fclose(fileChild1) == -1){
        error("closing fileChild1");
        printf("gothere\n");
        return -1;
    }
    if (fclose(fileChild2) == -1){
        error("closing fileChild2");
        printf("gothere\n");
        return -1;
    }
    return 0;

}

void waitForChildren(child *child1, child *child2){
    int status1, status2;

    while (waitpid(child1->pid, &status1, 0) == -1){
        free(child1);
        free(child2);
        error("waitpid for child1");
        exit(1);
    }
    while (waitpid(child2->pid, &status2, 0) == -1){
        free(child1);
        free(child2);
        error("waitpid for child2");
        exit(1);
    }

}