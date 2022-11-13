/**
* @file forksort.c
* @author Filip Coja <e12023141@student.tuwien.ac.at>
* @date 09.12.2021
*
* @brief The program takes multiple lines as input and sorts them by using a recursive variant of merge sort.
* @details The input is read from stdin and ends when an EOF (End Of File) is encountered.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/** Error codes defined as constants */
#define PIPE_FAILED -1
#define FORK_FAILED -2
#define EXEC_FAILED -3
#define DUP_FAILED  -4

/** Struct used for storing child information */
typedef struct {
    int pid;
    int stdin_pipe[2];
    int stdout_pipe[2];
} child_t;

/**
 * @brief Initializes pipes used for IPC.
 * @details Makes the pipes ready for being used for a child struct
 * Error handling is also implemented.
 *
 * @param child The child to which the pipe should be opened to.
 * @return Status Code. 1 means success. PIPE_FAILED means there was an error creating the pipe.
 */
int init_pipe(child_t *child) {
    // 0 => read, 1 => write
    if (pipe(child->stdin_pipe) == -1) {
        return PIPE_FAILED;
    }
    if (pipe(child->stdout_pipe) == -1) {
        close(child->stdin_pipe[0]);
        close(child->stdin_pipe[1]);
        return PIPE_FAILED;
    }
    return 1;
}

/**
 * @brief Creates a fork and executes the child process.
 * @details Initializes pipe with init_pipe() and forks process. Closes unnecessary write or read ends. Redirects stdin
 * and stdout according to implementation.
 *
 * @param child The child to be executed.
 * @return Status code, only if parent process, child process ends with execution of new process.
 */
int run_child(child_t *child) {
    /** Error opening pipe */
    if (init_pipe(child) == PIPE_FAILED) {
        return PIPE_FAILED;
    }

    /** Fork process */
    int pid = fork();
    if (pid == -1) return FORK_FAILED;

    /** Check if parent or child process */
    if (pid == 0) {
        close(child->stdin_pipe[1]);
        close(child->stdout_pipe[0]);

        if (dup2(child->stdin_pipe[0], STDIN_FILENO) == -1) {
            return DUP_FAILED;
        }
        if (dup2(child->stdout_pipe[1], STDOUT_FILENO) == -1) {
            return DUP_FAILED;
        }
        if (execlp("./forksort", "forksort", NULL) == -1) {
            return EXEC_FAILED;
        }
    } else {
        close(child->stdin_pipe[0]);
        close(child->stdout_pipe[1]);
    }
    return pid;
}

/**
 * @brief Checks if line from stdin is greater than one.
 * @details Depending on result, line1 and line2 are filled with getline() info if there are greater than one lines.
 *
 * rewind() works for the parent but not for the children processes and because of this isn't used in here.
 *
 * If 1 returned for > 1 lines, line1 and line2 need to be freed.
 *
 * @param line1
 * @param line2
 * @return Status code, 0 for exactly one line, -2 for 0 lines, 1 for > 1 lines, and -1 for errors.
 */
int greater_one_line(char **line1, char **line2) {
    int line_c = 0;

    // getline stuff
    size_t buff_size = 0;
    char *buff = NULL;
    while (true) {
        long read = getline(&buff, &buff_size, stdin);
        if (read == -1) {
            if (line_c == 1) {
                free(buff);
                return 0;
            }
            if (line_c < 1) {
                free(buff);
                return -2;
            }
        }

        if (line_c >= 1) {
            /** Normaler case wenn >1 lines */
            *line2 = malloc(read + 1);
            if (*line2 == NULL) {
                free(buff);
                return -1;
            }
            strcpy(*line2, buff);
            free(buff);
            return 1;
        }

        *line1 = malloc(read + 1);
        if (*line1 == NULL) {
            free(buff);
            return -1;
        }
        strcpy(*line1, buff);
        line_c++;
    }
}

/**
* @brief Main entry point
* @details Main function. Options are created and default settings are set.
*
* @param argc
* @param argv
* @return exit code
*/
int main(int argc, char **argv) {
    if (argc > 1) {
        fprintf(stderr, "No arguments allowed. \nUsage: ./forksort");
        exit(EXIT_FAILURE);
    }

    /** Check if more than one line in stdin */
    char *line1 = NULL;
    char *line2 = NULL;
    int status = greater_one_line(&line1, &line2);
    if (status == -1) {
        fprintf(stderr, "Allocation Error");
        exit(EXIT_FAILURE);
    } else if (status == 0) {
        fprintf(stdout, "%s", line1);
        free(line1);
        exit(EXIT_SUCCESS);
    } else if (status == -2) {
        exit(EXIT_SUCCESS);
    }

    /** Only executed if more than one line */
    /** Run children processes */
    child_t child_1;
    child_t child_2;
    if ((child_1.pid = run_child(&child_1)) < 0) {
        fprintf(stderr, "Error running child 1");
        exit(EXIT_FAILURE);
    }
    if ((child_2.pid = run_child(&child_2)) < 0) {
        fprintf(stderr, "Error running child 2");
        exit(EXIT_FAILURE);
    }

    /** Open write pipes to children */
    FILE *f1 = fdopen(child_1.stdin_pipe[1], "w");
    if (f1 == NULL) {
        fprintf(stderr, "Error opening file descriptors for writing to children.");
        exit(EXIT_FAILURE);
    }
    FILE *f2 = fdopen(child_2.stdin_pipe[1], "w");
    if (f2 == NULL) {
        fprintf(stderr, "Error opening file descriptors for writing to children.");
        exit(EXIT_FAILURE);
    }

    /** Split and write stdin to each child alternately */
    fprintf(f1, "%s", line1);
    fprintf(f2, "%s", line2);
    free(line1);
    free(line2);

    int line_count = 2;
    size_t buff_size = 0;
    char *buff = NULL;
    while (getline(&buff, &buff_size, stdin) != -1) {
        if (line_count % 2 == 0) {
            fprintf(f1, "%s", buff);
        } else {
            fprintf(f2, "%s", buff);
        }
        line_count++;
    }
    free(buff);
    /** Close write pipes */
    fclose(f1);
    fclose(f2);

    /** Wait for child processes to finish */
    waitpid(child_1.pid, &status, 0);
    if (WEXITSTATUS(status) == EXIT_FAILURE) {
        close(child_1.stdout_pipe[0]);
        close(child_2.stdout_pipe[0]);
        fprintf(stderr, "Error in child process 1. Exiting.. \n");
        exit(EXIT_FAILURE);
    }
    waitpid(child_2.pid, &status, 0);
    if (WEXITSTATUS(status) == EXIT_FAILURE) {
        close(child_1.stdout_pipe[0]);
        close(child_2.stdout_pipe[0]);
        fprintf(stderr, "Error in child process 2. Exiting.. \n");
        exit(EXIT_FAILURE);
    }

    /** Open stdout from children */
    FILE *child_f1 = fdopen(child_1.stdout_pipe[0], "r");
    if (child_f1 == NULL) {
        close(child_1.stdout_pipe[0]);
        close(child_2.stdout_pipe[0]);
        fprintf(stderr, "Error opening file descriptor f1. Exiting..");
        exit(EXIT_FAILURE);
    }
    FILE *child_f2 = fdopen(child_2.stdout_pipe[0], "r");
    if (child_f2 == NULL) {
        close(child_1.stdout_pipe[0]);
        close(child_2.stdout_pipe[0]);
        fprintf(stderr, "Error opening file descriptor f2. Exiting..");
        exit(EXIT_FAILURE);
    }

    /** Merge sub parts */
    size_t first_buff_size = 0;
    char *first_buff = NULL;
    size_t second_buff_size = 0;
    char *second_buff = NULL;

    bool skip_first = false;
    bool skip_second = false;
    long read1 = -1;
    long read2 = -1;
    while (true) {
        if (!skip_first) {
            read1 = getline(&first_buff, &first_buff_size, child_f1);
            first_buff[strcspn(first_buff, "\n")] = '\0';
            skip_first = false;
        }
        if (!skip_second) {
            read2 = getline(&second_buff, &second_buff_size, child_f2);
            second_buff[strcspn(second_buff, "\n")] = '\0';
            skip_second = false;
        }

        if (read1 == -1 && read2 == -1) {
            break;
        } else if (read1 == -1) {
            fprintf(stdout, "%s\n", second_buff);
            skip_first = true;
            skip_second = false;
            continue;
        } else if (read2 == -1) {
            fprintf(stdout, "%s\n", first_buff);
            skip_second = true;
            skip_first = false;
            continue;
        }

        if (strcmp(first_buff, second_buff) < 0) {
            fprintf(stdout, "%s\n", first_buff);
            skip_second = true;
            skip_first = false;
        } else {
            fprintf(stdout, "%s\n", second_buff);
            skip_first = true;
            skip_second = false;
        }
    }
    free(first_buff);
    free(second_buff);

    /** Cleanup resources */
    fclose(child_f1);
    fclose(child_f2);

    return EXIT_SUCCESS;
}
