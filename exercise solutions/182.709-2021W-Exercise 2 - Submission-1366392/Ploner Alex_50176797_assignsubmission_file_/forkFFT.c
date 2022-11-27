/*
 * @file forkFFT.c
 * @author Alex Ploner 12024704
 * @date 06.12.2021
 *
 * @brief fork fourier transformation in c
 *
 * @details implements the Cooley-Turkey Fast Fourier Transformation algorithm using
 *          fork, pipes and execlp
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "forkFFT.h"
#include <math.h>

char *program_name;


/*
 * @brief The main logic of the forkFTT program
 * @details Reads at first the input form stdin and checks whether the input contains more than one number.
 *          If that is the case, then two child process and two pipes are created. The parent than writes to one child all
 *          numbers with even indices and to the other all number with odd indices.
 *          If the input contains exactly one number, this number is printed to the pipe or to stdout.
 *          Once the child process are terminated, the parent process reads the partial results from the pipes and calculates the result
 * @param argc Argument counter
 * @param argv Arguments
 * @return EXIT_SUCCESS upon success
 *         and EXIT_FAILURE otherwise.
 */
int main(int argc, char **argv) {

    program_name = argv[0];

    if (argc > 1) {
        fprintf(stderr, "[%s] USAGE: %s\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    char buf_even[BUFFER_SIZE];
    char buf_odd[BUFFER_SIZE];

    // read input
    if (fgets(buf_even, BUFFER_SIZE, stdin) == NULL) {
        fprintf(stderr,"[%s] ERROR: Error while reading input: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fgets(buf_odd, BUFFER_SIZE, stdin) == NULL) {
        fprintf(stdout, "%s\n", buf_even);
        exit(EXIT_SUCCESS);
    }

    //DONE: implement 3) onwards
    int vk_1[2], kv_1[2];
    if (pipe(vk_1) == -1) {
        fprintf(stderr, "[%s] ERROR: unable to create pipe: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pipe(kv_1) == -1) {
        fprintf(stderr, "[%s] ERROR: unable to create pipe: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid_t pids_2;
    pid_t pids_1 = fork();
    if (pids_1 == -1) {
        fprintf(stderr, "[%s] ERROR: failed to create child\n", program_name);
        exit(EXIT_FAILURE);
    } else if (pids_1 == 0) {
        // first child
        close(vk_1[1]);
        close(kv_1[0]);

        if (dup2(vk_1[0], STDIN_FILENO) == -1) {
            fprintf(stderr, "[%s] ERROR: unable to copy the file descriptor to STDIN: %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (dup2(kv_1[1], STDOUT_FILENO) == -1) {
            fprintf(stderr,"[%s] ERROR: unable to copy the file descriptor to STDOUT: %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        close(vk_1[0]);
        close(kv_1[1]);


        execlp(program_name, program_name, NULL);

        fprintf(stderr, "[%s] ERROR: unable to execute execlp for child 1: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);

    } else {
        // father

        int vk_2[2], kv_2[2];
        if (pipe(vk_2) == -1) {
            fprintf(stderr, "[%s] ERROR: unable to create pipe: %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (pipe(kv_2) == -1) {
            fprintf(stderr, "[%s] ERROR: unable to create pipe: %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        pids_2 = fork();
        if (pids_2 == -1) {
            fprintf(stderr, "[%s] ERROR: failed to create child 2\n", program_name);
            exit(EXIT_FAILURE);
        } else if (pids_2 == 0) {
            // second child.
            close(vk_1[0]);
            close(vk_1[1]);
            close(kv_1[0]);
            close(kv_1[1]);

            close(vk_2[1]);
            close(kv_2[0]);

            if (dup2(vk_2[0], STDIN_FILENO) == -1) {
                fprintf(stderr, "[%s] ERROR: unable to copy the file descriptor to STDIN: %s\n", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (dup2(kv_2[1], STDOUT_FILENO) == -1) {
                fprintf(stderr, "[%s] ERROR: unable to copy the file descriptor to STDOUT: %s\n", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }

            close(vk_2[0]);
            close(kv_2[1]);

            execlp(program_name, program_name, NULL);

            fprintf(stderr, "[%s] ERROR: unable to execute execlp for child 2: %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        } else {
            // father

            close(vk_1[0]);
            close(vk_2[0]);
            close(kv_1[1]);
            close(kv_2[1]);


            FILE *fileE;
            if ((fileE = fdopen(vk_1[1], "w")) == NULL) {
                close(kv_1[0]);
                close(kv_2[0]);
                fprintf(stderr, "[%s] ERROR: failed to open file descriptor: %s\n", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            FILE *fileO;
            if ((fileO = fdopen(vk_2[1], "w")) == NULL) {
                fclose(fileE);
                close(kv_1[0]);
                close(kv_2[0]);
                fprintf(stderr, "[%s] ERROR: failed to open file descriptor: %s\n", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }

            // read from stdin and write to children

            if (fputs(buf_even, fileE) == EOF) {
                fprintf(stderr, "[%s] ERROR: failed to write to child: %s\n", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }

            if (fputs(buf_odd, fileO) == EOF) {
                fprintf(stderr, "[%s] ERROR: failed to write to child: %s\n", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }

            int count = 2;
            while (fgets(buf_even, BUFFER_SIZE, stdin) != NULL) {
                if (fputs(buf_even, fileE) == EOF) {
                    fprintf(stderr, "[%s] ERROR: failed to write to child: %s\n", program_name, strerror(errno));
                    fclose(fileE);
                    fclose(fileO);
                    exit(EXIT_FAILURE);
                }

                if (fgets(buf_odd, BUFFER_SIZE, stdin) == NULL) {
                    fprintf(stderr, "[%s] ERROR: number of arguments not even\n", argv[0]);
                    fclose(fileE);
                    fclose(fileO);
                    exit(EXIT_FAILURE);
                }

                if (fputs(buf_odd, fileO) == EOF) {
                    fprintf(stderr, "[%s] ERROR: failed to write to child: %s\n", program_name, strerror(errno));
                    fclose(fileE);
                    fclose(fileO);
                    exit(EXIT_FAILURE);
                }
                count += 2;
            }

            fclose(fileO);
            fclose(fileE);

            //handle termination of children
            int status;
            waitpid(pids_1, &status, 0);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == EXIT_FAILURE) {
                    close(kv_1[0]);
                    close(kv_2[0]);
                    fprintf(stderr, "[%s] ERROR: child1 process failed\n", program_name);
                    exit(EXIT_FAILURE);
                }
            }
            waitpid(pids_2, &status, 0);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == EXIT_FAILURE) {
                    close(kv_1[0]);
                    close(kv_2[0]);
                    fprintf(stderr, "[%s] ERROR: child2 process failed\n", program_name);
                    exit(EXIT_FAILURE);
                }
            }


            generate_result(kv_1[0], kv_2[0], count);
        }
    }

    exit(EXIT_SUCCESS);
}

void generate_result(int c1, int c2, int length) {

    complex_t ro, re, result[length];
    FILE *e_fd, *o_fd;
    if ((e_fd = fdopen(c1, "r")) == NULL) {
        fprintf(stderr, "[%s] ERROR: failed to open file descriptor: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((o_fd = fdopen(c2, "r")) == NULL) {
        fprintf(stderr, "[%s] ERROR: failed to open file descriptor: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char number[BUFFER_SIZE];
    for (int i = 0; i < length / 2; ++i) {


        if (fgets(number, BUFFER_SIZE, e_fd) == NULL) {
            fprintf(stderr, "[%s] ERROR: unable to read (child even)\n", program_name);
            exit(EXIT_FAILURE);
        }

        string_to_complex(number, &re);

        if (fgets(number, BUFFER_SIZE, o_fd) == NULL) {
            fprintf(stderr, "[%s] ERROR: unable to read (child odd)\n", program_name);
            exit(EXIT_FAILURE);
        }

        string_to_complex(number, &ro);

        butterfly_operation(&re, &ro, length, i);

        result[i] = re;
        result[i + length / 2] = ro;
    }

    fclose(e_fd);
    fclose(o_fd);

    for (int i = 0; i < length; ++i) {
        snprintf(number, BUFFER_SIZE, "%f %f*i\n", result[i].real, result[i].img);
        if (fputs(number, stdout) == EOF) {
            fprintf(stderr, "[%s] ERROR: writing solution to stdout", program_name);
            exit(EXIT_FAILURE);
        }
    }
}


void string_to_complex(char *number, complex_t *cnum) {

    char *end;
    errno = 0;
    cnum->real = strtof(number, &end);
    if (errno == ERANGE || (errno != 0 && cnum->real == 0)) {
        fprintf(stderr, "[%s] ERROR: failed parsing values: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (end == number) {
        fprintf(stderr, "[%s] ERROR: failed parsing values: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (*end == ' ') {
        cnum->img = strtof(end, &end);
        if (errno == ERANGE || (errno != 0 && cnum->real == 0)) {
            fprintf(stderr, "[%s] ERROR: failed parsing values: %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (end == number) {
            fprintf(stderr, "[%s] ERROR: failed parsing values: %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if ((strcmp("*i\n", end)) != 0) {
            fprintf(stderr, "[%s] ERROR: input does not end correct\n", program_name);
            exit(EXIT_FAILURE);
        }
    } else if (*end == '\n') {
        cnum->img = 0;
    } else {
        fprintf(stderr, "[%s] ERROR: invalid number\n", program_name);
        exit(EXIT_FAILURE);
    }

}

void butterfly_operation(complex_t *re, complex_t *ro, int n, int k) {
    complex_t c, temp;
    c.real = cos((-(2 * PI) / n) * k);
    c.img = sin((-(2 * PI) / n) * k);

    temp.real = c.real;
    temp.img = c.img;

    c.real = temp.real * ro->real - temp.img * ro->img;
    c.img = temp.real * ro->img + temp.img * ro->real;

    ro->real = re->real - c.real;
    ro->img = re->img - c.img;

    re->real += c.real;
    re->img += c.img;
}