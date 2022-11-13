/**
 * @file mygrep.c
 * @author Jonas Neumair <e11911064@student.tuwien.ac.at>
 * @date 01.11.2021
 *
 * @brief Main program module.
 * 
 * This program 
 **/
#include "forkFFT.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <complex.h>
#include <math.h>

static char *pgm_name; /* The program name. */

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void) {
	(void) fprintf(stderr, "USAGE: %s\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Error function.
 * @brief This function writes error messages to stderr.
 * @details Function prints message and exits program.
 */
static void error(char *msg) {
	(void) fprintf(stderr, "Error %s: %s\n", pgm_name, msg);
	exit(EXIT_FAILURE);
}

/**
 * @brief Calculates cooley-tukey fast fourier transform for given number.
 * 
 */
void calculate_fourier(int eRead, int oRead, int size) {
    // fd to read from
    FILE *evenFile = fdopen(eRead, "r");
    FILE *oddFile = fdopen(oRead, "r");
    // variables for saving real and imaginery values
    float eReal, oReal;
    float eImg, oImg;
    // array of results
    float complex result[size/2];
    // input line
    char line_str[MAX_LINE_LENGTH];
    // checks if strtol only consists of floats
    char *endptr = NULL;

    for (int k=0; k < size/2; k++) {
        // read evenFile
        if (fgets(line_str, MAX_LINE_LENGTH, evenFile) == NULL) {
            fclose(evenFile);
            fclose(oddFile);
            error("Cannot read even file!");
        }
        // first float in line
        eReal = strtof(line_str, &endptr);
        // check endptr, if empty, second value exists
        if (*endptr==' ') {
            eImg = strtof(endptr, &endptr);
            if (strcmp("*i\n", endptr) != 0) {
                error("EPart Imaginery number missing!");
            }
        } else if (*endptr == '\n') {
            eImg = 0;
        } else {
            fclose(evenFile);
            fclose(oddFile);
            error("Cannot transform even part to float");
        }

        // read oddFile
        if (fgets(line_str, MAX_LINE_LENGTH, oddFile) == NULL) {
            fclose(evenFile);
            fclose(oddFile);
            error("Cannot read odd file!");
        }
        // first float in line
        oReal = strtof(line_str, &endptr);
        // check endptr, if empty, second value exists
        if (*endptr==' ') {
            oImg = strtof(endptr, &endptr);
            if (strcmp("*i\n", endptr) != 0) {
                error("OPart Imaginery number missing!");
            }
        } else if (*endptr == '\n') {
            oImg = 0;
        } else {
            fclose(evenFile);
            fclose(oddFile);
            error("Cannot transform odd part to float");
        }
        // calculation
        float complex a = cos(-(2*PI*k)/size); 
        float complex b = sin(-(2*PI*k)/size);
        float complex c = oReal;
        float complex d = oImg;
        float complex rechterTeil = a*c - b*d + I*(a*d + b*c);
        
        float complex k_result = (eReal+eImg*I) + rechterTeil;
        //print first half now and only save second half -> more memory efficient
        fprintf(stdout, "%.6f %+.6f*i\n", creal(k_result), cimag(k_result));
        result[k] = (eReal+eImg*I) - rechterTeil;
    }
    fclose(evenFile);
    fclose(oddFile);

    for (int i = 0; i < size/2; i++) {
        fprintf(stdout, "%.6f %+.6f*i\n", creal(result[i]), cimag(result[i]));
    }
}

void fork_child(child_t *child) {
    // reading and writing to child
    int read_pipe_child[2];
    int write_pipe_child[2];
    if ( (pipe(read_pipe_child) < 0) || (pipe(write_pipe_child) < 0)) {
        error("Cannot open pipes!");
    }

    child->pid = fork();

    switch (child->pid) {
        case -1:
            error("Cannot fork!");
            break;

        case 0:
            // child process
            // close
            close(read_pipe_child[1]);
            close(write_pipe_child[0]);
            if ((dup2(read_pipe_child[0], STDIN_FILENO) < 0) || (dup2(write_pipe_child[1], STDOUT_FILENO) < 0)) {
                error("Cannot redirect stdin or stdout!");
            }
            close(read_pipe_child[0]);
            close(write_pipe_child[1]);
            // execute main
            execlp(pgm_name, pgm_name, NULL);
            error("Cannot execute program");
            break;

        default:
            close(read_pipe_child[0]);
            close(write_pipe_child[1]);
            child->read_pipe = write_pipe_child[0];
            child->write_pipe = read_pipe_child[1];
            break;
    }  
}

/**
 * Program entry point.
 * @brief The program starts here.
 * @details
 * 
 * global variables: pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv)
{
    pgm_name = argv[0];
    int result_size = 0;

    if (argc > 1) {
        usage();
    }

    // first 2 numbers
    char buffer_even[MAX_LINE_LENGTH];
    char buffer_odd[MAX_LINE_LENGTH];

    if (fgets(buffer_even, MAX_LINE_LENGTH, stdin) == NULL) {
        error("Cannot read stdin!");
    }

    // if only one number is provided, we stop before creating children
    if (fgets(buffer_odd, MAX_LINE_LENGTH, stdin) == NULL) {
        fprintf(stdout, "%s", buffer_even);
        return EXIT_SUCCESS;
    }

    // even part fd
    child_t ePart;
    // odd part fd
    child_t oPart;

    // create children
    fork_child(&ePart);
    fork_child(&oPart);

    FILE *eFile = fdopen(ePart.write_pipe, "w");
    FILE *oFile = fdopen(oPart.write_pipe, "w");
    
    if (eFile == NULL || oFile == NULL) {
        error("Cannot open files!");
    }

    // print to childs
    if (fprintf(eFile, "%s", buffer_even) < 0) {
        fclose(eFile);
        fclose(oFile);
        error("Cannot write to even part!");
    };
    if (fprintf(oFile, "%s", buffer_odd) < 0) {
        fclose(eFile);
        fclose(oFile);
        error("Cannot write to odd part!");
    };
    // increase result size every time
    result_size += 2;

    // read rest of the file
    while (fgets(buffer_even, MAX_LINE_LENGTH, stdin) != NULL) {
        if (fprintf(eFile, "%s", buffer_even) < 0) {
            error("Cannot write to even part!");
        };

        if (fgets(buffer_odd, MAX_LINE_LENGTH, stdin) != NULL) {
            if (fprintf(oFile, "%s", buffer_odd) < 0) {
                error("Cannot write to odd part!");
            };
        } else {
            // close resource if only one number is given
            fclose(eFile);
            fclose(oFile);
            close(ePart.read_pipe);
            close(oPart.read_pipe);
            error("Cannot process uneven input!");
        }
        result_size += 2;
    }
    //close files after finished reading and writing
    fclose(eFile);
    fclose(oFile);

    //checking status of child pids
    int status;
    // waiting for even part child
    waitpid(ePart.pid, &status, 0);
    // normal termination
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != EXIT_SUCCESS) {
            // close both if one fails
            close(ePart.read_pipe);
            close(oPart.read_pipe);
            error("Epart Child failed");
        }
    }

    //waiting for uneven part child
    waitpid(oPart.pid, &status, 0);
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != EXIT_SUCCESS) {
            // close both if one fails
            close(ePart.read_pipe);
            close(oPart.read_pipe);
            error("Opart Child failed");
        }
    }
    //result_size = result_size-34000;
    //call calculation
    calculate_fourier(ePart.read_pipe, oPart.read_pipe, result_size);

    return EXIT_SUCCESS;
}
