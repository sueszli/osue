/**
 * @file forksort.c
 * @author Elias Pinter <e12023962@student.tuwien.ac.at>
 * @date 20.11.2021
 * @brief Program forksort: This program takes multiple lines as input and sorts them using a special implementation of mergesort
 * where dividing and merging is performed by multiple processes simultaniously.
 * @details If there is only one line as input, there is nothing to be sorted and the program prints it and terminates. Otherwise 
 * the process forks itself twice and writes half the lines to the first child process and the other half to the second child process.
 * this happens recursively until a child process only reads a single line then the child processes start sorting and writing the sorted
 * output to their parent processes. The first parent process then recieves the complete sorted input and prints it to the console.
 * The communication between processes is realised usinf pipes.
 **/


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static char* pgm_name;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name: the program name
 */
static void usage(void){
    (void)fprintf(stderr, "USAGE: %s \n", pgm_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief error handling
 * @details prints the error message which corresponds to errno to stderr.
 *          global variables: pgm_name: the program name
 */        
static void printError(void){
    fprintf(stderr, "%s: %s\n", pgm_name, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief divides the input into two parts
 * @details sends one half of the lines read to the first child process and the other half to the second child process
 * @param lines: number of lines that have been read.
 * @param lineArray: String array which contains each line read as an own entry.
 * @param firstChildWrite: FILE* which points to the write end of the first divide pipe.
 * @param secondChildWrite: FILE* which points to the write end of the second divide pipe.
 */

static void divide(int lines, char** lineArray, FILE* firstChildWrite, FILE* secondChildWrite){
    size_t child1Lines = lines / 2;

    for (int i = 0; i < child1Lines; i++) {
        fprintf(firstChildWrite, "%s", lineArray[i]);
    }

    for (int i = child1Lines; i < lines; i++) {
        fprintf(secondChildWrite, "%s", lineArray[i]);
    }
}

/**
 * @brief merges the output of two child processes in a sorted way.
 * @details reads from both merge pipes and merges the inpu in a sorted way. The sorted result is being printed to stdout, so its either
 * redirected to the next parent process or visible on the terminal if the first process executes this function
 * @param firstChildRead: FILE* which points to the read end of the first merge pipe.
 * @param secondChildRead: FILE* which points to the read end of the second merge pipe.
 */

static void merge(FILE* firstChildRead, FILE* secondChildRead){
    char* string1 = NULL;
    char* string2 = NULL;
    char* stringChosen;
    int compare = 0;
    size_t buf_size = 0;

    while (1 == 1) {
        buf_size = 0;

        if (string1 == NULL) {
            getline(&string1, &buf_size, firstChildRead);
        }

        buf_size = 0;

        if (string2 == NULL) {
            getline(&string2, &buf_size, secondChildRead);
        }

        if (strlen(string1) == 0 && strlen(string2) == 0) {
            break;
        }

        if (strlen(string1) == 0) {
            stringChosen = string2;
            string2 = NULL;
            fprintf(stdout, "%s", stringChosen);
            continue;
        }
        if (strlen(string2) == 0) {
            stringChosen = string1;
            string1 = NULL;
            fprintf(stdout, "%s", stringChosen);
            continue;
        }

        compare = strcmp(string1, string2);

        if (compare > 0) {
            stringChosen = string2;
            string2 = NULL;
        } else if (compare < 0) {
            stringChosen = string1;
            string1 = NULL;
        } else {
            stringChosen = string1;
            string1 = NULL;
        }
        fprintf(stdout, "%s", stringChosen);
    }
    free(string1);
    free(string2);
}
/**
 * @brief counts number of lines written to stdin.
 * @details Waits for the user to enter iniput and then reads it line for line from stdin 
 * and writes the read lines to lineArray.
 * @param lineArrayAdress char*** which points to the address of lineArray.
 * @return returns the number of lines it reads from stdin
 */
static int numOfLines(char*** lineArrayAdress){
    int lines = 0;
    char* buffer = NULL;
    size_t buf_size = 0;
    size_t line_length = 0;
    char** lineArray = *lineArrayAdress;
    int index = 0;

    while ((line_length = getline(&buffer, &buf_size, stdin)) != EOF) {
        lineArray = realloc(lineArray, (index + 1) * sizeof(char*));

        if (lineArray == NULL) {
            printError();
        }

        char* line = malloc(line_length + 1); // line_length doesnt include null terminate

        if (line == NULL) {
            printError();
        }
        strcpy(line, buffer);
        lineArray[index] = line;
        index++;
        lines++;
    }
    free(buffer);

    *lineArrayAdress = lineArray;
    return lines;
}

/**
 * @brief closes unused pipeends of a child process.
 * @details for the first child process this function closes:
 *          - write end of first divide pipe
            - read end of first merge pipe
            - write end of second divide pipe
            - read end of second merge pipe

            for the second child process this function closes:
            - read end of second merge pipe 
            - write end of second split pipe 
            - write end of first split pipe
            - read end of first merge pipe

 * @param ownDivideWr: file descriptor describing the write end of the child's own divide pipe.
 * @param ownMergeRd: file descriptor describing the read end of the child's own merge pipe.
 * @param otherDivideWr: file descriptor describing the write end of the other child's divide pipe.
 * @param otherMergeRd: file descriptor describing the read end of the other child's merge pipe.
 */
static void pipeHandlingChildren(int ownDivideWr, int ownMergeRd, int otherDivideWr, int otherMergeRd){
    int owdwr = close(ownDivideWr);
    int owmrd = close(ownMergeRd);
    int otdwr = close(otherDivideWr);
    int otmrd = close(otherMergeRd);

    if (owdwr == -1 || owmrd == -1 || otdwr == -1 || otmrd == -1) {
        printError();
    }
}

/**
 * @brief closes unused pipeends of a parent process.
 * @details this function closes:
 *          - read end of first divide pipe
            - read end of second divide pipe
            - write end of first merge pipe
            - write end of second merge pipe

 * @param firstDivideRd: file descriptor describing the read end of the first divide pipe.
 * @param secondDivideRd: file descriptor describing the read end of the second divide pipe.
 * @param firstMergeWr: file descriptor describing the write end of the frist merge pipe.
 * @param secondMergeWr: file descriptor describing the write end of the second merge pipe.
 */
static void pipeHandlingParent(int firstDivideRd, int secondDivideRd, int firstMergeWr, int secondMergeWr){
    int fdrd = close(firstDivideRd);
    int sdrd = close(secondDivideRd);
    int fmwr = close(firstMergeWr);
    int smwr = close(secondMergeWr);

    if (fdrd == -1 || sdrd == -1 || fmwr == -1 || smwr == -1) {
        printError();
    }
}

/**
 * @brief handles creation of child processes and redirection of stdin/stdout. Also takes care of correct communication order between
 * parent and child.
 * @details this function first creates all four pipes used for communication betwween processes. Aftewards the prent process 
 * forks itself for the first time to create the first child process. Frist child process then closes its unused pipe ends and redirects
 * stdin to read end of frist divide pipe and stdout to write end of frist merge pipe. After that the first child process 
 * executes the program. Meanwhile parent process forks itself for the second time to create second child process. Second child process 
 * then closes its unused pipe ends and redirects stdin to read end of second divide pipe and stdout to write end of second merge pipe. 
 * After that the second child process executes the program. Meanwhile parent closes its unused pipeends and starts sending input to the
 * childprocesses. Subsequently the parent starts reading the respective sorted solutions from his children merges them and writes them to
 * stout. At the end the parent process waits until both of his child processes terminate with EXIT_SUCCESS.
 * @param lines: number of lines the parent process read from stdin
 * @param lineArray: String array which contains each line read as an own entry.
 */

static void forkParent(int lines, char** lineArray){

    int pipeDivide1[2];
    int pipeDivide2[2];
    int pipeMerge1[2];
    int pipeMerge2[2];
    pipe(pipeDivide1);
    pipe(pipeDivide2);
    pipe(pipeMerge1);
    pipe(pipeMerge2);

    pid_t pid1 = fork();

    switch (pid1) {
    // first fork error
    case -1:
        printError();
        break;

    // first child tasks
    case 0:
        // closes writeEnd of first divide pipe for the first child
        // closes readEnd of first merge pipe for the first child
        // closes writeEnd of second divide pipe for the first child
        // closes readEnd of second merge pipe for the second child
        pipeHandlingChildren(pipeDivide1[1], pipeMerge1[0], pipeDivide2[1], pipeMerge2[1]);

        // redirects first childs stdin to readEnd of first divide pipe
        dup2(pipeDivide1[0], STDIN_FILENO);

        // redirects first childs stdout to writeEnd of first merge pipe
        dup2(pipeMerge1[1], STDOUT_FILENO);

        // restarts the program for the first child
        int ch1start = execlp(pgm_name, pgm_name, NULL);

        if (ch1start == -1) {
            printError();
        }

        break;

    // parent tasks
    default: {
        pid_t pid2 = fork();

        switch (pid2) {
        // second fork error
        case -1:
            printError();
            break;

        // second child tasks
        case 0:
            // closes readEnd of second merge pipe for the second Child
            // closes writeEnd of second divide pipe for the second Child
            // closes writeEnd of first divide pipe for the second child
            // closes readEnd of first merge pipe for the second child
            pipeHandlingChildren(pipeDivide2[1], pipeMerge2[0], pipeDivide1[1], pipeMerge1[1]);

            // redirects the second childs stdin to readEnd of second divide pipe
            dup2(pipeDivide2[0], STDIN_FILENO);

            // redirects the second childs stdout to write End of second merge pipe
            dup2(pipeMerge2[1], STDOUT_FILENO);

            int ch2start = execlp(pgm_name, pgm_name, NULL);

            if (ch2start == -1) {
                printError();
            }

            break;

        // parent tasks
        default:

            pipeHandlingParent(pipeDivide1[0], pipeDivide2[0], pipeMerge1[1], pipeMerge2[1]);

            FILE* firstChildWrite = fdopen(pipeDivide1[1], "w");
            FILE* secondChildWrite = fdopen(pipeDivide2[1], "w");

            if (firstChildWrite == NULL || secondChildWrite == NULL) {
                printError();
            }

            divide(lines, lineArray, firstChildWrite, secondChildWrite);

            int fcfcwr = fclose(firstChildWrite);
            int fcscwr = fclose(secondChildWrite);

            if (fcfcwr == EOF || fcscwr == EOF) {
                printError();
            }

            FILE* firstChildRead = fdopen(pipeMerge1[0], "r");
            FILE* secondChildRead = fdopen(pipeMerge2[0], "r");

            if (firstChildRead == NULL || secondChildRead == NULL) {
                printError();
            }

            merge(firstChildRead, secondChildRead);

            int fcfcrd = fclose(firstChildRead);
            int fcscrd = fclose(secondChildRead);

            if (fcfcrd == EOF || fcscrd == EOF) {
                printError();
            }

            int status = NULL;
            pid_t firstChild = waitpid(pid1, &status, 0);

            if (status != EXIT_SUCCESS) {
                fprintf(stderr, "%s: - %s\n", pgm_name, "first child failed");
                exit(EXIT_FAILURE);
            }

            pid_t secondChild = waitpid(pid2, &status, 0);

            if (status != EXIT_SUCCESS) {
                fprintf(stderr, "%s: - %s\n", pgm_name, "second child failed");
                exit(EXIT_FAILURE);
            }

            if (firstChild == -1 || secondChild == -1) {
                printError();
            }
        }
        break;
    }
    }
}

/**
 * Program entry point.
 * @brief The program starts here.
 * @details
 * The program doesn't take any options or arguments. It calls numOfLines and after that starts the recursive fork.
 * At the end all memory allocated to store the read lines is freed and the program terminates.
 * global variables: pgm_name: the program name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS on normal termination and EXIT_FAILURE otherwise.
 */
int main(int argc, char** argv){
    pgm_name = argv[0];

    if (argc != 1) {
        usage();
    }

    char** lineArray = NULL;

    int lines = numOfLines(&lineArray);
    if (lines == 1) {
        fprintf(stdout, "%s", lineArray[0]);
        free(lineArray[0]);
        free(lineArray);
        return EXIT_SUCCESS;
    }

    forkParent(lines, lineArray);

    for (int i = 0; i < lines; i++) {
        free(lineArray[i]);
    }
    free(lineArray);
    return EXIT_SUCCESS;
}