/**
 * @file main.c
 * @author Paul Brunner | 11919163
 * 
 * @brief this program expands input from stdin or input files,
 * wherever it reads in '\t', it replaces tabs with n spaces (n is given in option -t);
 * Option -o if printing the result in an output file. Else stdout.
 * 
 * @details At first program main.c handles options. 
 * First option: -t x; x el. natural numbers; 
 *  If option t is hit, every '\t' get replaced with |x| spaces.
 * Second option: -o xyz; xyz = String
 *  If option o is hit, the manupulated input (file/stdin) is printed in an output file named xyz;
 * 
 * Following, the program copies all input files, if any given, in an File array. If there is no
 * input file, stdin is set on input_files[0]
 * 
 * If option o is hit, a file named xyz is opened and the programm overwrites charakter for charakter
 * from input (input file/stdin) in the outputfile. If a charakter == '\t', |x| spaces replace the tab.
 * 
 * If option o is not hit, instead of printing it in an output file, its printed to the console stdout.
 * @version 0.1
 * @date 2021-11-11
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

extern int errno;

/**
 * @brief main
 * 
 * @param argc is the value of arguments given 
 * @param argv array of argzments. Size = argc - 1
 * @return int 0 if succed
 */
int main(int argc, char *argv[]) {
    
    /**
     * int opt_counter: how many options are given. Starting with one because of program name
     * int c: value of getopt
     * int spaces: how many spaces are replacing '\t'
     * int outfileBool: true if option -o is hit
     * char name_outfile[]
     * If case '?' is hit, a wrong option letter is given.[]: name of output file
     * char *placeholder: necessary for strtol; no other usage
     */
    int opt_counter = 1;
    int c = 0;
    int spaces = 8;
    int outfileBool = 0;
    char name_outfile[1000] = "";
    char *placeholder = NULL; 

    /**
     * @brief handles options
     * @details 
     * First option: -t x; x el. natural numbers; 
     *      If option t is hit, every '\t' get replaced with |x| spaces;
     *      Number x is converted from string to int spaces.
     * Second option: -o xyz; xyz = String
     *      If option o is hit, the manupulated input (file/stdin) is printed in an output file named xyz;
     *      The outfile name is copied to name_outfile.
     * If case '?' is hit, a wrong option letter is given.
     */
    while ((c = getopt(argc, argv, "t:o:")) != -1) {
        switch (c) {
            case 't' : 
                if (optarg != NULL) {
                    opt_counter += 2;
                    if ((spaces = strtol(optarg, &placeholder, 10)) == 0) {
                        printf("%s", "ERROR: option -t needs an integer as argument\n");
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            case 'o' : 
                if (optarg != NULL) {
                    opt_counter += 2;
                    outfileBool = 1;
                    strcpy(name_outfile, optarg);
                }
                break;
            case '?' :
                printf("ERROR: this option is not valid\n"); 
                exit(EXIT_FAILURE);
                break;
        }
    }

    /**
     * int files: number of input files;
     * int arg_index: number of arguments before the first input file
     * FILE *input_files[]: array of files
     * int i: for-loop
     */
    int files = argc - opt_counter;
    int arg_index = opt_counter;
    FILE *input_files[(files == 0 ? 1 : files)];
    int i = 0;

    /**
     * @brief copies input files in an array
     * @details
     * If any input files exist, this for loop copies them in *input_files[]
     * If not, stdin is set on index 0
     */
    if (files != 0) {
        for (i = 0; i < files; i++) {
            if ((input_files[i] = fopen(argv[arg_index], "r")) == NULL) {
                fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", "main" , strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    } else {
        input_files[0] = stdin;
        files = 1;
    }

    /**
     * @brief open output file and copies characters
     * @details
     * At first if option -o is given, opens output file with name xyz.
     * Else, the handler *fp is set to stdout
     * 
     * int char_readin: 
     * First for loop is for every input file or stdin
     * do {} while (1) is to copy every charakter with getc() from input_file[i]
     * and putc() in outputflie or stdout.
     * Second for loop replaces the '\t' charakters with |spaces| * ' '.
     * At the end close all files. 
     */
    FILE *fp;
    if (outfileBool) {
        fp = fopen(name_outfile, "w+");
    } else {
        fp = stdout;
    }

    int char_readin = 0;
    for (i = 0; i < files; i++) {
        do {
            char_readin = fgetc(input_files[i]);

            if (feof(input_files[i])) {
                break;
            }
            if (char_readin == '\t') {
                int j = 0;
                for (j = 0; j < spaces; j++) {
                    fputc(' ', fp);
                }
            }
            else {
                fputc(char_readin, fp);
            }
        } while (1);
        fclose(input_files[i]);
    }
    fclose(fp);
}
