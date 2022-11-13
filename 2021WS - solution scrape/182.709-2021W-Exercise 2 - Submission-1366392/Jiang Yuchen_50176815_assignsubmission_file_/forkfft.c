/**
 * @file forkfft.c
 * @author Jiang Yuchen 12019845
 * @date 14.11.2021
 * @brief forkFFT program
 **/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <regex.h>
#include "forkfft.h"

/**
 * Handles the signal SIGPIPE
 * @brief Overrides the exit
 * @param signal signal
 */
void handle_signal(int signal) {
}

/**
 * Method for multiplying 2 imaginary numbers
 * @param i1 First number
 * @param i2 Second number
 * @return Calculated Number
 */
static Imaginary multi(Imaginary *i1, Imaginary *i2) {
    float a = i1->re;
    float b = i1->im;
    float c = i2->re;
    float d = i2->im;
    Imaginary val = {.re = a*c - b*d, .im = a*d + b*c};
    return val;
}

/**
 * Main method for calculating the Fourier Transform
 * @param argc Stores the number of arguments (unused)
 * @param argv Array of arguments (unused)
 * @return EXIT_SUCCESS
 */
int main(int argc, char** argv) {

    // Signals
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handle_signal;
    sigaction(SIGPIPE, &sa, NULL);

    char *line = NULL;
    char *imPart = NULL;
    char *endptr = NULL;
    size_t bufsize = 0;
    Imaginary values;
    Imaginary number;
    int forked = 0;
    int vSize = 0;

    pid_t pid1;
    pid_t pid2;
    FILE *child1;
    FILE *child2;

    int pipefd1_in[2];
    int pipefd1_out[2];
    int pipefd2_in[2];
    int pipefd2_out[2];

    regex_t regex;
    int r = regcomp(&regex, "^(-?[0-9]+(\\.[0-9]+)?)( -?[0-9]+(\\.[0-9]+)?)?(\n)?$", REG_EXTENDED);
    if (r) {
        printf("Regex compile failed\n");
        exit(EXIT_FAILURE);
    }

    while (getline(&line, &bufsize, stdin) != -1) {
        // regex match
        r = regexec(&regex, line, 0, NULL, 0);
        if (r != 0) {
            regfree(&regex);
            fprintf(stderr, "[%s] Wrong format (Regex match failed)\n", argv[0]);
            if (line != NULL) {
                free(line);
            }
            exit(EXIT_FAILURE);
        }

        // Store re part
        errno = 0;
        number.re = strtof(line, &imPart);
        if (errno == ERANGE) {
            fprintf(stderr, "[%s] Value too large: %s\n", argv[0], strerror(errno));
            if (line != NULL) {
                free(line);
            }
            regfree(&regex);
            exit(EXIT_FAILURE);
        }
        // Store im part
        number.im = strtof(imPart, &endptr);
        if (errno == ERANGE) {
            fprintf(stderr, "[%s] Value too large: %s\n", argv[0], strerror(errno));
            if (line != NULL) {
                free(line);
            }
            regfree(&regex);
            exit(EXIT_FAILURE);
        }

        // Store number at the end
        if (vSize % 2 == 0) {
            values = number;
            vSize++;
        } else {
            vSize++;
            if (forked == 0) {
                forked = 1;
                // child 1 in
                if (pipe(pipefd1_in) == -1) {
                    fprintf(stderr, "[%s], pipe failed: %s\n", argv[0], strerror(errno));
                    regfree(&regex);
                    exit(EXIT_FAILURE);
                }
                // child 1 out
                if (pipe(pipefd1_out) == -1) {
                    fprintf(stderr, "[%s], pipe failed: %s\n", argv[0], strerror(errno));
                    regfree(&regex);
                    exit(EXIT_FAILURE);
                }
                // split
                pid1 = fork();
                if (pid1 == -1) {
                    fprintf(stderr, "[%s] fork failed: %s\n", argv[0], strerror(errno));
                    regfree(&regex);
                    exit(EXIT_FAILURE);
                }
                if (pid1 == 0) {
                    regfree(&regex);
                    // child 1
                    close(pipefd1_in[1]);
                    close(pipefd1_out[0]);
                    if (dup2(pipefd1_in[0], STDIN_FILENO) == -1) {
                        fprintf(stderr, "[%s] dup2 failure: %s\n", argv[0], strerror(errno));
                        close(pipefd1_in[0]);
                        close(pipefd1_out[1]);
                        exit(EXIT_FAILURE);
                    }
                    close(pipefd1_in[0]);
                    if (dup2(pipefd1_out[1], STDOUT_FILENO) == -1) {
                        fprintf(stderr, "[%s] dup2 failure: %s\n", argv[0], strerror(errno));
                        close(pipefd1_out[1]);
                        exit(EXIT_FAILURE);
                    }
                    close(pipefd1_out[1]);
                    execl("./forkFFT", "forkFFT", NULL);
                } else {
                    // close unused pipes of parent
                    close(pipefd1_in[0]);
                    close(pipefd1_out[1]);

                    // child 2 in
                    if (pipe(pipefd2_in) == -1) {
                        fprintf(stderr, "[%s], pipe failed: %s\n", argv[0], strerror(errno));
                        regfree(&regex);
                        exit(EXIT_FAILURE);
                    }

                    // child 2 out
                    if (pipe(pipefd2_out) == -1) {
                        fprintf(stderr, "[%s], pipe failed: %s\n", argv[0], strerror(errno));
                        regfree(&regex);
                        exit(EXIT_FAILURE);
                    }

                    // split
                    pid2 = fork();
                    if (pid2 == -1) {
                        fprintf(stderr, "[%s] fork failed: %s\n", argv[0], strerror(errno));
                        regfree(&regex);
                        exit(EXIT_FAILURE);
                    }
                    if (pid2 == 0) {
                        regfree(&regex);
                        // child 2
                        close(pipefd2_in[1]);
                        close(pipefd2_out[0]);
                        if (dup2(pipefd2_in[0], STDIN_FILENO) == -1) {
                            fprintf(stderr, "[%s] dup2 failure: %s\n", argv[0], strerror(errno));
                            close(pipefd2_in[0]);
                            close(pipefd2_out[1]);
                            exit(EXIT_FAILURE);
                        }
                        close(pipefd2_in[0]);
                        if (dup2(pipefd2_out[1], STDOUT_FILENO) == -1) {
                            fprintf(stderr, "[%s] dup2 failure: %s\n", argv[0], strerror(errno));
                            close(pipefd2_out[1]);
                            exit(EXIT_FAILURE);
                        }
                        close(pipefd2_out[1]);
                        execl("./forkFFT", "forkFFT", NULL);
                    } else {
                        // close unused pipes of parent
                        close(pipefd2_in[0]);
                        close(pipefd2_out[1]);

                        child1 = fdopen(pipefd1_in[1], "w");
                        child2 = fdopen(pipefd2_in[1], "w");

                        // write values into pipe (child 1 = even, child 2 = odd)
                        fprintf(child1, "%f %f\n", values.re, values.im);
                        fprintf(child2, "%f %f\n", number.re, number.im);
                    }
                }
            } else {
                // write values into pipe (child 1 = even, child 2 = odd)
                fprintf(child1, "%f %f\n", values.re, values.im);
                fprintf(child2, "%f %f\n", number.re, number.im);
            }
        }
    }
    regfree(&regex);

    // free line
    if (line != NULL) {
        free(line);
        line = NULL;
    }

    if (vSize == 1) {
        printf("%f %f\n", number.re, number.im);
        exit(EXIT_SUCCESS);
    }
    if (vSize == 0) {
        fprintf(stderr, "[%s] No numbers (Usage: forkFFT NUMBERS)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // close fdopen
    fclose(child1);
    fclose(child2);

    if (vSize % 2 == 1) {
        fprintf(stderr, "[%s] Not even (Usage: forkFFT NUMBERS)\n", argv[0]);
        close(pipefd1_out[0]);
        close(pipefd2_out[0]);
        exit(EXIT_FAILURE);
    }

    // check child status
    int wstatus1;
    int wstatus2;
    if (waitpid(pid1, &wstatus1, 0) == -1) {
        close(pipefd1_out[0]);
        close(pipefd2_out[0]);
        exit(EXIT_FAILURE);
    }
    if (waitpid(pid2, &wstatus2, 0) == -1) {
        close(pipefd1_out[0]);
        close(pipefd2_out[0]);
        exit(EXIT_FAILURE);
    }
    if (WEXITSTATUS(wstatus1) == EXIT_SUCCESS && WEXITSTATUS(wstatus2) == EXIT_SUCCESS) {
        Imaginary op[vSize];
        int counter = 0;

        // read from pipe of child 1
        FILE *pipe1 = fdopen(pipefd1_out[0], "r");
        char *buf_even = NULL;
        bufsize = 0;
        imPart = NULL;
        while (getline(&buf_even, &bufsize, pipe1) != -1) {
            errno = 0;
            op[counter].re = strtof(buf_even, &imPart);
            if (errno == ERANGE) {
                fprintf(stderr, "[%s] Value too large: %s\n", argv[0], strerror(errno));
                close(pipefd1_out[0]);
                close(pipefd2_out[0]);
                free(buf_even);
                fclose(pipe1);
                exit(EXIT_FAILURE);
            }
            op[counter].im = strtof(imPart, NULL);
            if (errno == ERANGE) {
                fprintf(stderr, "[%s] Value too large: %s\n", argv[0], strerror(errno));
                close(pipefd1_out[0]);
                close(pipefd2_out[0]);
                free(buf_even);
                fclose(pipe1);
                exit(EXIT_FAILURE);
            }
            counter++;
        }
        free(buf_even);
        fclose(pipe1);

        // read from pipe of child 2
        FILE *pipe2 = fdopen(pipefd2_out[0], "r");
        char *buf_odd = NULL;
        bufsize = 0;
        imPart = NULL;
        while (getline(&buf_odd, &bufsize, pipe2) != -1) {
            errno = 0;
            op[counter].re = strtof(buf_odd, &imPart);
            if (errno == ERANGE) {
                fprintf(stderr, "[%s] Value too large: %s\n", argv[0], strerror(errno));
                close(pipefd1_out[0]);
                close(pipefd2_out[0]);
                free(buf_odd);
                fclose(pipe2);
                exit(EXIT_FAILURE);
            }
            op[counter].im = strtof(imPart, NULL);
            if (errno == ERANGE) {
                fprintf(stderr, "[%s] Value too large: %s\n", argv[0], strerror(errno));
                close(pipefd1_out[0]);
                close(pipefd2_out[0]);
                free(buf_odd);
                fclose(pipe2);
                exit(EXIT_FAILURE);
            }
            counter++;
        }
        free(buf_odd);
        fclose(pipe2);

        // calculate and write
        Imaginary q;
        int half = vSize / 2;
        for (int i = 0; i < half; ++i) {
            q.re = cosf((-2.0f * (float) M_PI * (float) i) / (float) vSize);
            q.im = sinf((-2.0f * (float) M_PI * (float) i) / (float) vSize);
            q = multi(&q, &op[half + i]);
            if (printf("%f %f\n", op[i].re + q.re, op[i].im + q.im) < 0) {
                close(pipefd1_out[0]);
                close(pipefd2_out[0]);
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < half; ++i) {
            q.re = cosf((-2.0f * (float) M_PI * (float) i) / (float) vSize);
            q.im = sinf((-2.0f * (float) M_PI * (float) i) / (float) vSize);
            q = multi(&q, &op[half + i]);
            if (printf("%f %f\n", op[i].re - q.re, op[i].im - q.im) < 0) {
                close(pipefd1_out[0]);
                close(pipefd2_out[0]);
                exit(EXIT_FAILURE);
            }
        }
        close(pipefd1_out[0]);
        close(pipefd2_out[0]);
        exit(EXIT_SUCCESS);
    } else {
        fprintf(stderr, "[%s] child not successful\n", argv[0]);
        close(pipefd1_out[0]);
        close(pipefd2_out[0]);
        exit(EXIT_FAILURE);
    }
}
