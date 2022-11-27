/**
 * @file forkFFT.c
 * @author Stefan Theuerer <e01228985@student.tuwien.ac.at>
 * @brief The program computes the Fourier Transform of its intput values.
 * @details the programm works recursively by using fork(2) and execlp(3)
 * @date 08.12.2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <complex.h>
#include <errno.h>
#include <math.h>

#define PI 3.141592654 /* approximation of pi */


char *myprog; /**< The program name */

/**
 * @brief prints the correct usage of the programm (forkFFT)
 * @details global variables: forkFFT
 */
void usage(void) {
    (void) fprintf(stderr,"Usage: %s\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief exit programm with 1 and prints error message
 * @details prints an given message and a errorstring
 *          if an error number is given before exit
 *          the programm with EXIT_FAILURE
 * @param message to print on stderr
 */
void exit_error(char* message) {
    if (errno == 0) {
        (void) fprintf(stderr,"Error: %s\n", message);
    } else {
        (void) fprintf(stderr,"Error: %s [%s]\n", message, strerror(errno));
    }   
    exit(EXIT_FAILURE);
}

/**
 * @brief parse a float number and gets
 * real part and imaginary part of number
 * @param number float number as string
 * @param real pointer on the real part of number
 * @param imag pointer on the imaginary part of number
 * @return int -1 if number is invalid or not a number
 *              2 if it is a valid number
 */
int parseFloat(char *number, float *real, float *imag){
    char *endptr, *endptr2;

    if(*number == '\n') {
        return -1;
    }

    if(isalpha(*number)) {
        return -1;
    }

    *real = strtof(number, &endptr);
    if(*endptr == '\n') { // real float
        return 2;
    } 

    if(isalpha(*endptr++)) {
        return -1;
    }

    *imag = strtof(endptr, &endptr2);
    if(strcmp(endptr2, "*i\n") == 0) { // imag float
       return 2;
    }
    return -1;
}

/**
 * @brief reads complex numbers from a given filedescriptor
 * 
 * @param buf buffer for the complex numbers
 * @param fd filedescriptor to read from
 */
void readFromFD(float complex *buf, int fd) {

    char *endptr, *endptr2;
    char *number = NULL;
    FILE *input = fdopen(fd, "r");
    size_t len = 0;
    ssize_t line_size;
    float real, imag;

    int i = 0;
    while((line_size = getline(&number,&len,input)) != -1) {
        real = strtof(number, &endptr);
        imag = strtof(endptr, &endptr2);
        buf[i++] = (float complex) real+imag*I;
    } 
}

/**
 * @brief main programm witch is calling itself recursivly
 * to read in float numbers and calculate the fourier transformation 
 * @param argc argument counter which must be one
 * @param argv argument array which must be empty
 * @return int EXIT_SUCCESS on success or
 *             EXIT_FAILURE on failure
 */
int main(int argc, char **argv) {
    myprog = argv[0];
    int ch;
    while ((ch = getopt(argc, argv, "")) != -1) {
        switch (ch)
        {
        case '?':
            usage();
        default:
            assert(0);
        }
    }

    if(argc > 1) {
        usage();
    }

    char *number = NULL;
    int amount = 0;
    int ret = 0;
    size_t len = 0;
    ssize_t line_size;
    float real = 0.0;
    float imag = 0.0;
    float complex c_buf[2];

    pid_t pid_odd;
    pid_t pid_even;

    int pipefd1[2]; /* first child stdout to parent */
    int pipefd2[2]; /* parent to stdin from first child */
    int pipefd3[2]; /* second child stdout to parent */
    int pipefd4[2]; /* parent to stdin from second child */

    if (pipe(pipefd1) < 0){
        exit_error("creating pipe C1->P failed");
    }
    if (pipe(pipefd2) < 0){
        exit_error("creating pipe P->C1 failed");
    }
    if (pipe(pipefd3) < 0){
        exit_error("creating pipe C2->P failed");
    }
    if (pipe(pipefd4) < 0){
        exit_error("creating pipe P->C2 failed");
    }

    while((line_size = getline(&number, &len, stdin)) != -1) {
        if((ret = parseFloat(number, &real, &imag)) == -1) {
            exit_error("Invalid input");
        }

        c_buf[amount%2] = (float complex) real+imag*I;
        amount++;
        // write numbers to children processes
        if(amount % 2 == 0 && amount > 2){
            dprintf(pipefd2[1], "%f %f*i\n", crealf(c_buf[0]), cimagf(c_buf[0]));
            dprintf(pipefd4[1], "%f %f*i\n", crealf(c_buf[1]), cimagf(c_buf[1]));
        }
        // start recursion if amount is higher than 1
        if(amount == 2) {
            pid_even = fork();
            
            switch (pid_even)
            {
            case -1:
                exit_error("Even fork failed");
            case 0: // child task
                if(dup2(pipefd1[1], STDOUT_FILENO) == -1) {
                    exit_error("dup2(pipefd1[0], STDOUT_FILENO) failed");
                }
                if(dup2(pipefd2[0], STDIN_FILENO) == -1) {
                    exit_error("dup2(pipefd2[0], STDIN_FILENO) failed");
                }
                if(close(pipefd1[0]) == -1) {
                    exit_error("close(pipefd1[1]) failed");
                }
                if(close(pipefd1[1]) == -1) {
                    exit_error("close(pipefd1[0]) failed");
                }
                if(close(pipefd2[0]) == -1) {
                    exit_error("close(pipefd2[0]) failed");
                }
                if(close(pipefd2[1]) == -1) {
                    exit_error("close(pipefd2[1]) failed");
                }
                if(close(pipefd3[0]) == -1) {
                    exit_error("close(pipefd3[0]) failed");
                }
                if(close(pipefd3[1]) == -1) {
                    exit_error("close(pipefd3[1]) failed");
                }
                if(close(pipefd4[0]) == -1) {
                    exit_error("close(pipefd4[0]) failed");
                }
                if(close(pipefd4[1]) == -1) {
                    exit_error("close(pipefd4[1]) failed");
                }
                
                execlp("./forkFFT", "forkFFT", NULL);
                // shouldn't be reached
                exit_error("execlp on even child failed");
            default: // parent task
                if(close(pipefd2[0]) == -1) {
                    exit_error("close(pipefd2[0]) failed");
                }
                dprintf(pipefd2[1], "%f %f*i\n", crealf(c_buf[0]), cimagf(c_buf[0]));

                pid_odd = fork();
                switch (pid_odd)
                {
                case -1:
                    exit_error("Odd fork failed");
                case 0: // child task
                    if(dup2(pipefd3[1], STDOUT_FILENO) == -1) {
                        exit_error("dup2(pipefd3[1], STDOUT_FILENO) failed");
                    }
                    if(dup2(pipefd4[0], STDIN_FILENO) == -1) {
                        exit_error("dup2(pipefd4[0], STDIN_FILENO) failed");
                    }
                    if(close(pipefd1[0]) == -1) {
                        exit_error("close(pipefd1[0]) failed");
                    }
                    if(close(pipefd1[1]) == -1) {
                        exit_error("close(pipefd1[1]) failed");
                    }
                    if(close(pipefd2[1]) == -1) {
                        exit_error("close(pipefd2[1]) failed");
                    }
                    if(close(pipefd3[0]) == -1) {
                        exit_error("close(pipefd3[0]) failed");
                    }
                    if(close(pipefd3[1]) == -1) {
                        exit_error("close(pipefd3[1]) failed");
                    }
                    if(close(pipefd4[0]) == -1) {
                        exit_error("close(pipefd4[0]) failed");
                    }
                    if(close(pipefd4[1]) == -1) {
                        exit_error("close(pipefd4[1]) failed");
                    }
                    
                    execlp("./forkFFT", "forkFFT", NULL);
                    // shouldn't be reached
                    exit_error("execlp on odd child failed");
                default: // parent
                    if(close(pipefd4[0]) == -1) {
                        exit_error("close(pipefd4[0]) failed");
                    }
                    dprintf(pipefd4[1], "%f %f*i\n", crealf(c_buf[1]), cimagf(c_buf[1]));
                }
            }
        }
    }

    if(amount == 1) {
        printf("%f %f*i\n", crealf(c_buf[0]), cimagf(c_buf[0]));
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
    if((amount % 2)!=0) {
        exit_error("Amount of input numbers is odd");
    }

    float complex *P_E = (float complex *)malloc(sizeof(float complex) * amount/2);
    float complex *P_O = (float complex *)malloc(sizeof(float complex) * amount/2);
    float complex *R = (float complex *)malloc(sizeof(float complex) * amount);

    if(close(pipefd1[1]) == -1) {
        exit_error("close(pipefd1[1]) failed");
    }
    if(close(pipefd2[1]) == -1) {
        exit_error("close(pipefd2[1]) failed");
    }
    if(close(pipefd3[1]) == -1) {
        exit_error("close(pipefd3[1]) failed");
    }
    if(close(pipefd4[1]) == -1) {
        exit_error("close(pipefd4[1]) failed");
    }

    int status1, status2;
    pid_t wait_status1, wait_status2;
    // wait for children termintate with sucess
    do {
        if((wait_status1 = waitpid(pid_even, &status1, 0)) == -1) {
            exit_error("waitpid even failed");
        }
        if(WEXITSTATUS(status1) == EXIT_FAILURE) {
            exit_error("exit even with failure");
        }
    } while(!WIFEXITED(status1));
    readFromFD(P_E, pipefd1[0]);
    if(close(pipefd1[0]) == -1) {
        exit_error("close(pipefd1[0]) failed");
    }

    do {
        if((wait_status2 = waitpid(pid_odd, &status2, 0)) == -1) {
            exit_error("waitpid odd failed");
        }
        if(WEXITSTATUS(status2) == EXIT_FAILURE) {
            exit_error("exit odd with failure");
        }
    } while(!WIFEXITED(status2));
    readFromFD(P_O, pipefd3[0]);
    if(close(pipefd3[0]) == -1) {
        exit_error("close(pipefd3[0]) failed");
    }

    /* DFT */
    for(int k = 0; k < amount/2; k++) {
        float complex result = (ccos(-((2*PI/amount)*k))+ csin(-((2*PI/amount)*k)) * I);

        R[k] = P_E[k] + result * P_O[k];
        R[k+(amount/2)] = P_E[k] - result * P_O[k];
    }

    for (int i = 0; i < amount; i++) {
        (void) fprintf(stdout,"%f %f*i\n", crealf(R[i]), cimagf(R[i]));
    }   
    fflush(stdout);

    free(R);
    free(P_E);
    free(P_O);

    return EXIT_SUCCESS;
}
