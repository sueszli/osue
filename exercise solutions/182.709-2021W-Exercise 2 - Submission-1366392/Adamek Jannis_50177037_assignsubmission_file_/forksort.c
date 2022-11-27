/**
 * @file forksort.c
 * @author Jannis Adamek (11809490)
 * @date 2021-12-05
 *
 * @brief sort lines by using a recursive merge sort algorithm with fork and
 * exec. Exercise 2.
 **/

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/** Used to distingue for pipe array */
#define READ 0
#define WRITE 1

/** The program reads two lines from stdin at the beginning. The first is called
 * LEFT and the second is called RIGHT.
 */
#define LEFT 0
#define RIGHT 1

/** Name of the executable file. (argv[0]) */
static char *program_name = NULL;

/**
 * Insure that there are no program arguments provided.
 * @param argc count of arguments including the program name
 * @param argv vector of program arguments
 * @brief Sets the global variable program_name and exits the program in case
 * that additional arguments are provided.
 */
static void parse_args(int argc, char *argv[]) {
    program_name = argv[0];
    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", program_name);
        exit(EXIT_FAILURE);
    }
}

/**
 * Print a useful error message and exit the program
 * @brief print the pid, program name, message and errno onto strerr
 * @param message a message that should descripe in what context the error
 * occurred.
 */
static void print_error_and_exit(char *message) {
    fprintf(stderr, "[%s] PID: %d %s Error: %s\n", program_name, getpid(),
            message, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * Open pipes, a wrapper for pipe() to check for errors.
 * @brief opens a pipe by using pipe() and it to the provided array
 * @param pipe_ptr array that will be filled if this function returns
 */
static void open_pipe(int pipe_ptr[2]) {
    if (pipe(pipe_ptr) == -1) {
        print_error_and_exit("pipe");
    }
}

/**
 * Fork the program, exec if child.
 * @brief Fork the program. The child execs a new copy of forksort and prepares
 * the pipes. pipe_this_to_parent will be mapped to stdout; pipe_parent_to_this
 * will be mapped to stdin. If the method returns, the following code is
 * executed by the parent.
 * @details If either fork or exec are unsuccessful, the program terminates.
 * @param child_id the pid of the child as returned by fork()
 * @param pipe_this_to_parent pipe from the created child to the parent
 * @param pipe_parent_to_this pipe from the parent to the child
 * @param pipe_other_to_parent pipe from the second child to the parent, will be
 * closed
 * @param pipe_parent_to_other pipe from the parent to the second child, will be
 * closed
 */
static void fork_and_prepare_pipes(pid_t *child_id, int pipe_this_to_parent[2],
                                   int pipe_parent_to_this[2],
                                   int pipe_other_to_parent[2],
                                   int pipe_parent_to_other[2]) {

    *child_id = fork();
    if (*child_id == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (*child_id == 0) {
        close(pipe_other_to_parent[READ]);
        close(pipe_other_to_parent[WRITE]);
        close(pipe_parent_to_other[READ]);
        close(pipe_parent_to_other[WRITE]);

        close(pipe_this_to_parent[READ]);
        close(pipe_parent_to_this[WRITE]);

        if (dup2(pipe_parent_to_this[READ], STDIN_FILENO) == -1) {
            print_error_and_exit("dup2 failed");
        }
        if (dup2(pipe_this_to_parent[WRITE], STDOUT_FILENO) == -1) {
            print_error_and_exit("dup2 failed");
        }
        close(pipe_parent_to_this[READ]);
        close(pipe_this_to_parent[WRITE]);

        execlp(program_name, program_name, NULL);

        // If the exec fails (=returns), we reach the following statements
        print_error_and_exit("Cannot exec");
        exit(EXIT_FAILURE);
    } else {
        close(pipe_this_to_parent[WRITE]);
        close(pipe_parent_to_this[READ]);
    }
}

/**
 * Block until the child exits.
 * @brief waits for the child with the pid child_pid and checks the return
 * status of that child. If the child did not exit successful, the parent exits
 * unsuccessfully as well.
 * @param child_pid the pid of the child
 */
static void wait_for_pid(pid_t child_pid) {
    int status;
    if (waitpid(child_pid, &status, 0) == -1) {
        print_error_and_exit("waitpid failed");
    }
    if (WEXITSTATUS(status) != EXIT_SUCCESS) {
        print_error_and_exit("status of child was not EXIT_SUCCESS");
    }
}

/**
 * Return the index of the line that comes first in the lines buffer
 * @brief Given a buffer of two lines, return the index of either the LEFT or
 * the RIGHT line, whoever comes first when sorted alphabetically.
 * @returns the index of the line, that comes first (either LEFT or RIGHT)
 */
static int decide_first(char *lines[2], bool lines_empty[2]) {
    if (lines_empty[LEFT]) {
        return RIGHT;
    }
    if (lines_empty[RIGHT]) {
        return LEFT;
    }
    if (strcmp(lines[LEFT], lines[RIGHT]) < 0) {
        return LEFT;
    }
    return RIGHT;
}

/**
 * Return true if the line ends with a newline character
 * @param line to be checked, this cannot be NULL
 */
static bool line_ends_with_newline(char *line) {
    return line[strlen(line) - 1] == '\n';
}

/**
 * Merge the returning lines from the children pipes.
 * @brief Until both pipe_left_to_parent and pipe_right_to_parent send EOF,
 * compare the two lines using decide_first() and print the winner to stdout.
 * @param pipe_left_to_parent pipe from the left child to the parent
 * @param pipe_right_to_parent pipe from the right child to the parent
 */
static void merge_from_children(int pipe_left_to_parent[2],
                                int pipe_right_to_parent[2]) {
    FILE *streams[2];
    streams[LEFT] = fdopen(pipe_left_to_parent[READ], "r");
    streams[RIGHT] = fdopen(pipe_right_to_parent[READ], "r");
    char *lines[2] = {NULL, NULL};
    size_t lens[2] = {0, 0};
    bool lines_exhausted[2] = {false, false};
    bool lines_empty[2] = {true, true};

    while (true) {
        for (int lr = LEFT; lr <= RIGHT; lr++) {
            if (lines_empty[lr] && !lines_exhausted[lr]) {
                int read = getline(lines + lr, lens + lr, streams[lr]);
                if (read == -1) {
                    lines_exhausted[lr] = true;
                    lines_empty[lr] = true;
                } else {
                    lines_empty[lr] = false;
                }
            }
        }
        if (lines_exhausted[LEFT] && lines_exhausted[RIGHT]) {
            break;
        }
        int winner_index = decide_first(lines, lines_empty);
        printf("%s", lines[winner_index]);
        lines_empty[winner_index] = true;
    }

    fclose(streams[LEFT]);
    fclose(streams[RIGHT]);
    free(lines[LEFT]);
    free(lines[RIGHT]);
}

/**
 * Alternatefly send lines from stdout to children
 * @brief Read lines from stdin and send them alternately to the left and right
 * child pipes.
 * @param pipe_parent_to_left pipe form left child to parent
 * @param pipe_parent_to_right pipe from right child to parent
 */
static void alternately_send_to_children(int pipe_parent_to_left[2],
                                         int pipe_parent_to_right[2]) {
    char *buffer = NULL;
    size_t buffer_len = 0;

    bool left_turn = true;
    while (getline(&buffer, &buffer_len, stdin) != -1) {
        if (left_turn) {
            write(pipe_parent_to_left[1], buffer, strlen(buffer));
            if (!line_ends_with_newline(buffer)) {
                write(pipe_parent_to_left[1], "\n", 1);
            }
        } else {
            write(pipe_parent_to_right[1], buffer, strlen(buffer));
            if (!line_ends_with_newline(buffer)) {
                write(pipe_parent_to_right[1], "\n", 1);
            }
        }
        left_turn = !left_turn;
    }
    free(buffer);
}

/**
 * Main
 * @brief Tries to reads two lines from stdin. If exactly one line was read, the
 * line gets printed, otherwise the program forks into two children and sends
 * the lines alternatively to them. After the children have exited, the program
 * merges the lines and prints them out to stdout.
 * @param argc number of program arguments
 * @param argv argument vector
 * @returns EXIT_SUCCESSFUL if everything with the parent and its children went
 * well otherwise EXIT_FAILURE
 */
int main(int argc, char *argv[]) {

    parse_args(argc, argv);

    char *lines[2] = {NULL, NULL};
    size_t lens[2] = {0, 0};

    if (getline(lines, lens, stdin) == -1) {
        free(lines[0]);
        exit(EXIT_SUCCESS);
    }

    if (getline(lines + 1, lens + 1, stdin) == -1) {
        printf("%s", lines[0]);
        free(lines[0]);
        free(lines[1]);
        exit(EXIT_SUCCESS);
    }

    int pipe_left_to_parent[2];
    int pipe_parent_to_left[2];
    int pipe_right_to_parent[2];
    int pipe_parent_to_right[2];

    open_pipe(pipe_left_to_parent);
    open_pipe(pipe_right_to_parent);
    open_pipe(pipe_parent_to_left);
    open_pipe(pipe_parent_to_right);

    pid_t pid_left;
    pid_t pid_right;

    fork_and_prepare_pipes(&pid_left, pipe_left_to_parent, pipe_parent_to_left,
                           pipe_right_to_parent, pipe_parent_to_right);

    fork_and_prepare_pipes(&pid_right, pipe_right_to_parent,
                           pipe_parent_to_right, pipe_left_to_parent,
                           pipe_parent_to_left);

    /* Parent code after fork starts here */

    // We have at least two lines, so we pipe the first line into the
    // left child and the second line into the right child.
    write(pipe_parent_to_left[WRITE], lines[0], strlen(lines[0]));
    write(pipe_parent_to_right[WRITE], lines[1], strlen(lines[1]));
    free(lines[0]);
    free(lines[1]);

    alternately_send_to_children(pipe_parent_to_left, pipe_parent_to_right);

    close(pipe_parent_to_left[WRITE]);
    close(pipe_parent_to_right[WRITE]);

    wait_for_pid(pid_left);
    wait_for_pid(pid_right);

    merge_from_children(pipe_left_to_parent, pipe_right_to_parent);

    close(pipe_left_to_parent[READ]);
    close(pipe_right_to_parent[READ]);

    exit(EXIT_SUCCESS);
}