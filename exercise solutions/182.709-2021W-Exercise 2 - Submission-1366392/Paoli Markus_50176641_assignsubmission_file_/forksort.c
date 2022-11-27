/**
 *  @file forksort.c
 *  @author Markus Paoli 01417212
 *  @date 2021/12/09
 *
 *  @brief The main module of the program "forksort".
 *
 *  This program reads lines from stdin and sorts them alphabeticaly by recursivley forking
 *  into child processes to perform a variant of merge sort.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

#include "utils.h"

/**
 * The error_exit function
 * @brief Prints error messages, then exits.
 *
 * @details This function prints custom error messages, including the program name,
 * to stderr. Then it calls exit(EXIT_FAILURE) to exit the program.
 * @param prog_name The program name.
 * @param err_msg The error message.
 */
void error_exit(const char *prog_name, char *err_msg) {
    fprintf(stderr, "[%s] Error: %s; %s.\n", prog_name, err_msg, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * The main function
 * @brief Handles the overall program flow, forking of child processes, communication with them, as well as errors.
 *
 * @details The main program function first checks for any arguments, then tries to read up to two lines from 
 * sdtin. If no lines where read and only EOF was encountered the program exits with an error. If one line 
 * of input was provided the function prints it to stdout, then exits succesfully. If more than two lines where
 * read four pies are set up and the programm forks two child processes. Each of the children redirects one of it's 
 * two pipes to stdin and stout respectivly, before calling execlp to recursivley execute this programm.
 * Meanwhile the parent continues to read lines from stdin and passes them on two the two child processes via pipes, alternating 
 * between the two. When there is no more input the parent reads input from the children again using pipes. It compares the lines
 * as it receives them using strcmp. Then prints them alphabetically in case sensitive order. Next the function uses waitpid to
 * evaluate the exit status of the two child processes. If any of them exits with EXIT_FAILURE the parent exits the same way.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS in case of success, or EXIT_FAILURE if an error occurred.
 */
int main(int argc, char **argv) {
    char *prog_name = argv[0];
    // there should be no options or arguments
    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", prog_name);
        exit(EXIT_FAILURE);
    }

    // try to read lines
    char *line_1 = read_line(STDIN_FILENO);
    if (line_1 == NULL) {
        error_exit(prog_name, "read_line failed");
    }

    //if (strcmp("", line_1) == 0) {
    //    free_line(line_1);
    //    fprintf(stderr, "[%s] Error: no input provided.\n", prog_name);
    //    exit(EXIT_FAILURE);
    //}

    char *line_2 = read_line(STDIN_FILENO);
    if (line_2 == NULL) {
        free_line(line_1);
        error_exit(prog_name, "read_line failed");
    }

    if (strcmp("", line_2) == 0) {
        if (write_line(STDOUT_FILENO, line_1) == -1) {
            free_line(line_1);
            free_line(line_2);
            error_exit(prog_name, "write_line failed");
        }
        close(STDOUT_FILENO);
        exit(EXIT_SUCCESS);
    }

    // open pipes for child 1
    int fds_to_1[2];
    if (pipe(fds_to_1) == -1) {
        free_line(line_1);
        free_line(line_2);
        error_exit(prog_name, "pipe failed");
    }
    int fds_from_1[2];
    if (pipe(fds_from_1) == -1) {
        free_line(line_1);
        free_line(line_2);
        error_exit(prog_name, "pipe failed");
    }

    // open pipes to child 2
    int fds_to_2[2];
    if (pipe(fds_to_2) == -1) {
        free_line(line_1);
        free_line(line_2);
        error_exit(prog_name, "pipe failed");
    }
    int fds_from_2[2];
    if (pipe(fds_from_2) == -1) {
        free_line(line_1);
        free_line(line_2);
        error_exit(prog_name, "pipe failed");
    }

    pid_t child_1, child_2;

    // fork child_1
    child_1 = fork();
    switch (child_1) {
        case -1:
            // fork error
            free_line(line_1);
            free_line(line_2);
            error_exit(prog_name, "fork failed");
            break;

        case 0:
            // child_1 code

            // close unneeded pipes
            close(fds_to_2[0]);
            close(fds_to_2[1]);
            close(fds_from_2[0]);
            close(fds_from_2[1]);

            // redirect pipes
            close(fds_to_1[1]);
            if (dup2(fds_to_1[0], STDIN_FILENO) != STDIN_FILENO) {
                free_line(line_1);
                free_line(line_2);
                error_exit(prog_name, "dup2 failed");
            }
            close(fds_to_1[0]);
            close(fds_from_1[0]);
            if (dup2(fds_from_1[1], STDOUT_FILENO) != STDOUT_FILENO) {
                free_line(line_1);
                free_line(line_2);
                error_exit(prog_name, "dup2 failed");
            }
            close(fds_from_1[1]);

            // execute forksort as child
            execlp(prog_name, prog_name, NULL);
            error_exit(prog_name, "execlp failed");
            break;

        default:
            // parent code

            // fork child_2
            child_2 = fork();
            switch (child_2) {
                case -1:
                    // fork error
                    free_line(line_1);
                    free_line(line_2);
                    error_exit(prog_name, "fork failed");
                    break;

                case 0:
                    // child_2 code

                    // close unneeded pipes
                    close(fds_to_1[0]);
                    close(fds_to_1[1]);
                    close(fds_from_1[0]);
                    close(fds_from_1[1]);

                    // redirect pipes
                    close(fds_to_2[1]);
                    if (dup2(fds_to_2[0], STDIN_FILENO) != STDIN_FILENO) {
                        free_line(line_1);
                        free_line(line_2);
                        error_exit(prog_name, "dup2 failed");
                    }
                    close(fds_to_2[0]);
                    close(fds_from_2[0]);
                    if (dup2(fds_from_2[1], STDOUT_FILENO) != STDOUT_FILENO) {
                        free_line(line_1);
                        free_line(line_2);
                        error_exit(prog_name, "dup2 failed");
                    }
                    close(fds_from_2[1]);

                    // execute forksort as child
                    execlp(prog_name, prog_name, NULL);
                    error_exit(prog_name, "execlp failed");
                    break;

                default:
                    // parent code

                    // close unneeded pipe ends
                    close(fds_to_1[0]);
                    close(fds_from_1[1]);
                    close(fds_to_2[0]);
                    close(fds_from_2[1]);

                    // write first two lines
                    if (write_line(fds_to_1[1], line_1) == -1) {
                        free_line(line_1);
                        free_line(line_2);
                        error_exit(prog_name, "write_line failed");
                    }
                    if (write_line(fds_to_2[1], line_2) == -1) {
                        free_line(line_1);
                        free_line(line_2);
                        error_exit(prog_name, "write_line failed");
                    }
                    free_line(line_1);
                    free_line(line_2);

                    // read lines from stdin (until EOF)
                    int i = 0;
                    char *line;
                    while (1) {
                        if ((line = read_line(STDIN_FILENO)) == NULL) {
                            error_exit(prog_name, "read_line failed");
                        }
                        // EOF reached
                        if (strcmp("", line) == 0) {
                            break;
                        }
                        // send to child_1
                        if (i % 2 == 0) {
                            if (write_line(fds_to_1[1], line) == -1) {
                                free_line(line);
                                error_exit(prog_name, "write_line failed");
                            }
                        }
                        // send to child_2
                        else {
                            if (write_line(fds_to_2[1], line) == -1) {
                                free_line(line);
                                error_exit(prog_name, "write_line failed");
                            }
                        }
                        free_line(line);
                        i = (i + 1) % 2;
                    }
                    free_line(line);
                    close(fds_to_1[1]);
                    close(fds_to_2[1]);

                    // receive lines, compare them and print to stdout
                    if ((line_1 = read_line(fds_from_1[0])) == NULL) {
                        error_exit(prog_name, "read_line failed");
                    }
                    if ((line_2 = read_line(fds_from_2[0])) == NULL) {
                        free_line(line_1);
                        error_exit(prog_name, "read_line failed");
                    }
                    while (1) {
                        if (strcmp(line_1, line_2) <= 0) {
                            if (write_line(STDOUT_FILENO, line_1) == -1) {
                                free_line(line_1);
                                free_line(line_2);
                                error_exit(prog_name, "write_line failed");
                            }
                            free_line(line_1);
                            if ((line_1 = read_line(fds_from_1[0])) == NULL) {
                                free_line(line_2);
                                error_exit(prog_name, "read_line failed");
                            }
                            if (strcmp("", line_1) == 0) {
                                while (strcmp("", line_2) != 0) {
                                    if (write_line(STDOUT_FILENO, line_2) == -1) {
                                        free_line(line_1);
                                        free_line(line_2);
                                        error_exit(prog_name, "write_line failed");
                                    }
                                    free_line(line_2);
                                    if ((line_2 = read_line(fds_from_2[0])) == NULL) {
                                        free_line(line_1);
                                        error_exit(prog_name, "read_line failed");
                                    }
                                }
                                break;
                            }
                        } else {
                            if (write_line(STDOUT_FILENO, line_2) == -1) {
                                free_line(line_1);
                                free_line(line_2);
                                error_exit(prog_name, "write_line failed");
                            }
                            free_line(line_2);
                            if ((line_2 = read_line(fds_from_2[0])) == NULL) {
                                free_line(line_1);
                                error_exit(prog_name, "read_line failed");
                            }
                            if (strcmp("", line_2) == 0) {
                                while (strcmp("", line_1) != 0) {
                                    if (write_line(STDOUT_FILENO, line_1) == -1) {
                                        free_line(line_1);
                                        free_line(line_2);
                                        error_exit(prog_name, "write_line failed");
                                    }
                                    free_line(line_1);
                                    if ((line_1 = read_line(fds_from_1[0])) == NULL) {
                                        free_line(line_2);
                                        error_exit(prog_name, "read_line failed");
                                    }
                                }
                                break;
                            }
                        }
                    }
                    close(STDOUT_FILENO);
                    close(STDIN_FILENO);
                    close(fds_from_1[0]);
                    close(fds_from_2[0]);
                    free_line(line_1);
                    free_line(line_2);

                    // wait for children to check exit status
                    int stat1, stat2;
                    pid_t pid1, pid2;
                    pid1 = waitpid(child_1, &stat1, 0);
                    pid2 = waitpid(child_2, &stat2, 0);
                    if (pid1 == -1 || pid2 == -1) {
                        error_exit(prog_name, "wait failed");
                    }
                    int failed1 = 1;
                    if (WIFEXITED(stat1) != 0) {
                        if (WEXITSTATUS(stat1) == EXIT_SUCCESS) {
                            failed1 = 0;
                        }
                    }
                    int failed2 = 1;
                    if (WIFEXITED(stat2) != 0) {
                        if (WEXITSTATUS(stat2) == EXIT_SUCCESS) {
                            failed2 = 0;
                        }
                    }
                    if (failed1 == 0 && failed2 == 0) {
                        exit(EXIT_SUCCESS);
                    } else {
                        error_exit(prog_name, "child exit failure");
                    }

                    break;
            }
            break;
    }
    assert(0);
}
