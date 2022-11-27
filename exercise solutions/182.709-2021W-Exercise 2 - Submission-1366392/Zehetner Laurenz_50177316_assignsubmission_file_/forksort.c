/**
 * @file forksort.c
 * 
 * @author Laurenz Zehetner 12023972
 * 
 * @brief This program sorts lines read from stdin alphabetically (ascending)
 * 
 * @details 
 * The program implements a special version of merge sort, 
 * where every part of the split list is sorted by a seperate child process created through fork 
 * and recursive execution of the program itself.
 * 
 * @date 11.12.2021
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define BUFSIZE 50

typedef struct {
    pid_t pid;
    FILE *in;
    FILE *out;
} child_node_t;


char *programName;

/**
 * @brief Prints Information about the correct use of this program.
 * @details global variables: programName
 * @param message additional information on the calling mistake to be printed
 */
static void usage(char *message) {
    fprintf(stderr, "[%s]: Usage: %s\n\t%s\n", programName, programName, message);
}

/**
 * @brief Exits EXIT_FAILURE and prints an error message to stderr
 * @details global variables: programName
 * @param message error message to be printed
 */
static void error_exit(char *message) {
    fprintf(stderr, "[%s]: %s\n", programName, message);
    exit(EXIT_FAILURE);
}


/**
 * @brief creates new child process and stores its input-pipe, output-pipe and pid in node
 * @details This function uses fork to create a child process and creates unnamed pipes to communicate with it.
 *  Parent Process:
 *      node.pid    stores the child-processes' pid
 *      node.in     stores a writeable stream to write to the child-processes' stdin
 *      node.out    stores a readable stream to read from the child-processes' stdout
 *      Unused pipe ends are closed
 * 
 *  Child Process:
 *      stdin is redirected to the pipe that can be written to by node.in in the parent process
 *      stdout is redirected to the pipe that can be read by node.out in the parent process
 *      Unused pipe ends are closed
 *      The programm calls itself recursively using execl
 * 
 * @param node after the call (in parent process): contains input-pipe, output-pipe and pid of child
 */
static void new_child(child_node_t *node) {
    int cp_pipe_fd[2];
    int pc_pipe_fd[2];

    if (pipe(pc_pipe_fd) == -1) error_exit("Creation of pipe (parent -> child) failed");
    if (pipe(cp_pipe_fd) == -1) error_exit("Creation of pipe (child -> parent) failed");

    pid_t pid = fork();
    if (pid == -1) error_exit("Fork failed");

    if (pid == 0) { //Child
        // Close unused pipe ends
        close(pc_pipe_fd[1]);   // Write end
        close(cp_pipe_fd[0]);   // Read end

        // Redirect used pipe ends
        dup2(pc_pipe_fd[0], STDIN_FILENO);  // Redirect stdin
        dup2(cp_pipe_fd[1], STDOUT_FILENO);  // Redirect stdout

        close(pc_pipe_fd[0]);   // Write end
        close(cp_pipe_fd[1]);   // Read end

        execl(programName, programName, NULL);
        error_exit("Exec Failed");

    } else {    // Parent
        node->pid = pid;

        // Close unused pipe ends
        close(pc_pipe_fd[0]);   // Read end
        close(cp_pipe_fd[1]);   // Write end

        node->in = fdopen(pc_pipe_fd[1], "w");
        node->out = fdopen(cp_pipe_fd[0], "r");

    }
}

/**
 * @brief Implements main functionality of forksort.c
 * @details 
 * Reads lines from stdin
 * If at least two lines have been read, the process creates two childprocesses and passes the read lines to them.
 * It passes the lines alternating between first and second childprocess.
 * After reading all lines from stdin, they are once again read from the childprocesses' .out streams, which are sorted by then.
 * It combines the two sorted streams by taking the lower (by strcmp) line read from the two streams and printing it to stdout.
 */
int main(int argc, char *argv[])
{
    programName = argv[0];
    if (argc > 1) {
        usage("Too many parameters");
        exit(EXIT_FAILURE);
    }

    char linebuffer[2][BUFSIZE];

    if (fgets(linebuffer[0], BUFSIZE, stdin) == NULL) {   // Empty input -> already sorted
        fclose(stdout);
        exit(EXIT_SUCCESS);
    }


    if (fgets(linebuffer[1], BUFSIZE, stdin) == NULL) {   // Only one line input -> already sorted
        fputs(linebuffer[0], stdout);
        fclose(stdout);
        exit(EXIT_SUCCESS);
    }


    // Stdin contained at least two lines -> split
    child_node_t child[2];

    new_child(&child[0]);
    new_child(&child[1]);

    // Input phase

    fputs(linebuffer[0], child[0].in);
    fputs(linebuffer[1], child[1].in);

    int splitter = 0;
    while (fgets(linebuffer[splitter], BUFSIZE, stdin) != NULL) {
        if(feof(stdin)) strncat(linebuffer[splitter], "\n", 1); // Ensure newline at the end of every line (case {'...', 'EOF'} -> {'...', '\n'})
        fputs(linebuffer[splitter], child[splitter].in);
        splitter = splitter == 0 ? 1 : 0;
    }

    // Close .in pipes -> EOF -> childs' 'Input phase' ends
    fclose(child[0].in);
    fclose(child[1].in);


    // Output phase
    // Read from both child .out streams until eof.
    // Compare the lines read from the streams and print the strcmp lower one to stdout.
    char *lines[2];
    lines[0] = fgets(linebuffer[0], BUFSIZE, child[0].out);
    lines[1] = fgets(linebuffer[1], BUFSIZE, child[1].out);

    while (lines[0] != NULL || lines[1] != NULL) {
        if (lines[splitter] != NULL) {
            if (lines[splitter ^ 1] != NULL) {
                if (strcmp(lines[splitter], lines[splitter ^ 1]) > 0) {
                    splitter = splitter == 0 ? 1 : 0;
                }
            }
            fputs(lines[splitter], stdout);
            lines[splitter] = fgets(linebuffer[splitter], BUFSIZE, child[splitter].out);
        } else {
            splitter = splitter == 0 ? 1 : 0;
        }
    }

    // Close .out pipes to avoid memory leak
    fclose(child[0].out);
    fclose(child[1].out);

    // Wait for child processes to exit
    int exit_status;

    waitpid(child[0].pid, &exit_status, 0);
    if (exit_status == EXIT_FAILURE) error_exit("Child exited with EXIT_FAILURE");

    waitpid(child[1].pid, &exit_status, 0);
    if (exit_status == EXIT_FAILURE) error_exit("Child exited with EXIT_FAILURE");

    return EXIT_SUCCESS;
}
