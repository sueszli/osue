/**
 * @file mygrep.c
 * @author Valentin Futterer 11904654
 * @date 01.11.2021
 * @brief Reads lines from input, if the contain a keyword, it prints them.
 * @details This Programm parses the arguments and opens corresponding files. Then it calls functions from
 * mygrepUtil.c to work with the inputs and write to the output.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mygrepUtil.h"

/**
 * @brief Main program body.
 * @details At first parses the arguments. Then it checks for the correct input, and opens the input and output files
 * (or stdin and stdout). It uses these files to call mygrep from mygrepUtil.c
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns 0 on success.
 */
int main(int argc, char* const argv[])
{
    char *output = NULL;
    int opt_o = 0;
    int opt_i = 0;
    int c;
    char *keyword;
    FILE *output_file = NULL;
    FILE *input_file = NULL;
    while ((c = getopt(argc, argv, "io:")) != -1 ) {
        switch (c) {
        case 'i': opt_i++;
        break;
        case 'o': output = optarg; opt_o++;
        break;
        case '?': usage(argv[0]);
        break;
        default: usage(argv[0]);
        break;
        }
    }

    // More than one caseinsensitive option
    if (opt_i > 1) {
        usage(argv[0]);
        
    }

    // more than one outfile option
    if (opt_o > 1) {
        usage(argv[0]);
    }

    // no keyword given
    if (argc - optind == 0) {
        usage(argv[0]);
    }
    keyword = argv[optind++];


    if(opt_o) {
        output_file = fopen(output, "w");
        if (output_file == NULL) {
            handle_error(argv[0], "Opening of the Output File failed");
        }
    // if there is no outfile option, the program copies the fd from stdout and opens it as a file
    } else {
        int cpy_stdout_fd = dup(1);
        if (cpy_stdout_fd == -1) {
            handle_error(argv[0], "Copying stdout file descriptor failed");
        }
        output_file = fdopen(cpy_stdout_fd, "w");
        if (output_file == NULL) {
            close(cpy_stdout_fd);
            handle_error(argv[0], "Opening of the Output to stdout failed");
        }
    }

    // if there is no inputfile argument, the program copies the fd from stdin and opens it as a file
    if (argc - optind == 0) {
        int cpy_stdin_fd = dup(0);
        if (cpy_stdin_fd == -1) {
            fclose(output_file);
            handle_error(argv[0], "Copying stdin file descriptor failed");
        }
        input_file = fdopen(cpy_stdin_fd, "r");
        if (input_file == NULL) {
            close(cpy_stdin_fd);
            fclose(output_file);
            handle_error(argv[0], "Opening of the Input to stdin failed");
        }
        if (mygrep(output_file, input_file, opt_i, keyword) != NULL) {
            fclose(output_file);
            fclose(input_file);
        }
    // loops over all inout files and calls mygrep for each one
    } else {
        while (argc - optind != 0) {
            input_file = fopen(argv[optind++], "r");
            if (input_file == NULL) {
                fclose(output_file);
                handle_error(argv[0], "Opening of the Input File failed");
            } 
            if (mygrep(output_file, input_file, opt_i, keyword) != NULL) {
                fclose(output_file);
                fclose(input_file);
            }
        }
    }
    if (fclose(output_file) != 0) {
        handle_error(argv[0], "Closing of Output file failed");
    }
    if (fclose(input_file) != 0) {
        handle_error(argv[0], "Closing of Input file failed");
    }
    return 0;
}
