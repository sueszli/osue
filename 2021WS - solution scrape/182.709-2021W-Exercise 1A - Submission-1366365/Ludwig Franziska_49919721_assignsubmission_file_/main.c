/**
 * @file main.c
 * @author Franziska Ludwig 01100784 <e1100784@student.tuwien.ac.at>
 * @date 31.10.2021
 *
 * @brief Main program module.
 * 
 * This program reads lines from files or from the command line and prints them to 
 * the command line or an output file, if the line contains a given keyword.
 **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

static char *program_name; 

/**
 * Usage function.
 * @brief This function prints information about the usage of the program in case of an error.
 * @details global variables: pogram_name
 */

static void usage(void){
    fprintf(stderr, "USAGE: %s [-i] [-o outfile] keyword [file...]\n", program_name);
	exit(EXIT_FAILURE);
}

/**
 * Sets all characters to lower case
 * @brief This function sets all characters of a string to lower case.
 * @param str The given string
*/

static void lower_string(char* str) {

    for(size_t i = 0; i < strlen(str); i++) {
        str[i] = tolower(str[i]);
    }

}

/**
 * Writes into an output file or on the command line.
 * @brief This function writes a string into an output file or to stdout.
 * @details If ouput is NULL, there is no output file and the line is printed
 * to stdout.
 * @param output Pointer to output file, NULL if there is none.
 * @param line String to be printed to output.
*/

static void createoutput(FILE *output, char *line) {

    if(output != NULL) {
        if((fputs(line, output)) == EOF) {
            fprintf(stderr, "fputs failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        } 
    } else {
        printf("%s", line);
    }
}

/**
 * Searches for keyword
 * @brief This function searches for the keyword in the command line or input file.
 * Reads lines until the end of a file or stdin is terminated and checks if they
 * match the keyword. If this is the case, calls createoutput.
 * @details Creates a copy of the read line, so it is possible to cover the case
 * case insensible option, but also to print the original line, if there is a match.
 * @param infile The input file or stdin 
 * @param outfile The output file or stdout
 * @param keyword The keyword string
 * @param iflag The int indicating case insensivity
*/

static void searchinput(FILE *infile, FILE *outfile, char *keyword, int iflag) {

    char *match, *line = NULL;
    char* line_cop = NULL;
    size_t length = 0;
    ssize_t read;

    while((read = getline(&line, &length, infile)) != -1) {

        if((line_cop = strdup(line)) == NULL){
            fprintf(stderr, "Stringcopy failed: %s\n", strerror(errno));
            EXIT_FAILURE;
        }
        if(iflag == 1){
            lower_string(line_cop);
        }

        if((match = (strstr(line_cop, keyword))) != NULL) {
            createoutput(outfile, line);
        }

        free(line_cop);
    }
    free(line);
}

/**
 * Entry point for the program
 * @brief Parses the given arguments and handles the different resulting cases.
 * Calls searchinput function to search for keyword in files or stdin.
 * @details global variable = program name.
 * Parses the arguments using getopt, handles wrong argument structure and 
 * calls the searchinput function depending on the input. Iterates over input files, if
 * given, or calls searchinput with stdin as file.  
 * @param argc The number of arguments 
 * @param argv[] Vector pointer containing arguments
 * @return Returns EXIT_SUCCESS on success
*/

int main(int argc, char *argv[]) {

    program_name = argv[0];

    FILE *output = NULL;
    FILE *input = NULL;
    char *filename;
    char *out_arg = NULL; 
    char *keyword = NULL;
    int counter; 
    int iflag = 0; 
    int file_position = 0;
    
    while ((counter = getopt(argc, argv, "io:")) != -1) {
        switch (counter) {
            case 'i':
                iflag = 1;
                break;
            case 'o':
                out_arg = optarg;
                break;
            case '?':
                if(optopt == 'o') {
                    fprintf(stderr, "Option -o requires an argument \n");
                    exit(EXIT_FAILURE);
                } else {
                    usage();    //Usage message is printed
                }
            default:
                break;
        }
    }

    file_position = optind + 1;   //Sets position to first filename

    if((argc - optind) <= 0) {    //No keyword argument given
        fprintf(stderr, "No non-option argument given \n");
        exit(EXIT_FAILURE);
    } else {
        keyword = argv[optind];    
    }

    if(iflag == 1) {
        lower_string(keyword);
    }

    if(out_arg != NULL) {       //True, when outfile is given as argument
        if((output = (fopen(out_arg, "w"))) == NULL) {
            fprintf(stderr, "fopen failed on output file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if((argc-optind) == 1) {    //True, when no input files given
        searchinput(stdin, output, keyword, iflag);
    } else {                   
        while((filename = argv[file_position]) != '\0') {
        if((input = fopen(filename, "r")) == NULL) {
            fprintf(stderr, "fopen failed on input file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        searchinput(input, output, keyword, iflag);
        fclose(input);
        file_position++;
    }
    }

    if(out_arg != NULL) {
        fclose(output);
    }

    return EXIT_SUCCESS;
}