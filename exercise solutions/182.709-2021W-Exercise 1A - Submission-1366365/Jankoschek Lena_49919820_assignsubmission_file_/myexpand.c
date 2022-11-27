/**
 * @file myexpand.c
 * @author Lena Jankoschek - 12019852
 * @brief a program which is a variation of the Unix-command expand
 * @version 0.1
 * @date 2021-11-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


/**
 * usage function
 * @brief this function prints the right usage of the program, its program name and exits the program with EXIT_FAILURE.
 * 
 * @param program_name - name of the program (argv[0])
 */
void usage(const char *program_name) {
    fprintf(stderr, "%s: Usage: [-t tabstop] [-o outfile] [file...]\n", program_name);
    exit(EXIT_FAILURE);
}


/**
 * is_digit function
 * @brief this function checks whether or not a pointer to a char is an integer.
 * 
 * @param arg - pointer to char (the argument of the option -t)
 * @return true - pointer to char is an integer
 * @return false - pointer to char is not an integer
 */
static bool is_digit(char *arg) {
    char *char_pointer = arg;
    int size = 0;
    //gets size of input
    while (*char_pointer != '\0') {
        size++;
        char_pointer++;
    }
    char arg_splitted[size];
    char_pointer = arg;
    //splits input -> so that 'abc' is ['a', 'b', 'c'] & checks if char is digit
    for(int i=0; i < size; i++) {
        sscanf(char_pointer, "%1s", &arg_splitted[i]);
        switch (arg_splitted[i]) {
        case '0':
            //01 is not a valid input
            if(i==0) {
                return false;
            }
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            //valid cases
            break;
        default:
            //an invalid character was parsed
            return false;
        }
        char_pointer++;
    }
    return true;
}


/**
 * parse_arguments function
 * @brief this function parses the arguments of the program and returns the number of input files
 * @details this function receives the program arguments (argc, argv) and parses them. If the arguments are invalid, it prints an error and 
 * the function usage is called. Aside from that, the function sets the path/name of the output file and sets the tabstop as character.
 * Parse_arguments returns the number of input files.
 * 
 * @param argc - argument counter
 * @param argv - argument values
 * @param output_filepath - NULL (parse_arguments sets the path/name of the outputfile in this parameter)
 * @param tabstop_c - NULL (parse agruments sets the tabstop, as a character, in this parameter)
 * @return int - returns the number of input files
 */
static int parse_arguments(int argc, char *argv[], char **output_filepath, char **tabstop_c) {
    int opt;
    const char *program_name = argv[0];
    //get/check options
    while ((opt = getopt(argc, argv, "t:o:")) != -1) {
        switch (opt) {
        case 't':
            //tabstop
            if (*tabstop_c != NULL) {
                fprintf(stderr, "%s: ERROR: flag -t can only appear once\n", program_name);
                usage(program_name);
            }
            if(!is_digit(optarg)) {
                fprintf(stderr, "%s: ERROR: tabstop must be an integer\n", program_name);
                usage(program_name);
            }
            *tabstop_c = optarg;
            break;
        case 'o':
            //outputfile
            if (*output_filepath != NULL) {
                fprintf(stderr, "%s: ERROR: flag -o can only appear once\n", program_name);
                usage(program_name);
            }
            *output_filepath = optarg;
            break;
        case '?':
            //at least one invalid option
            //fprintf(stderr, "[%s] ERROR: invalid option: %s\n", program_name, argv[optind-1]);
            usage(program_name);
            break;
        default:
            //unreachable
            assert(0);
        }
    }
    //return number of input files
    return  argc - optind;
}


/**
 * read_file function
 * @brief this funcion reads from the input file and writes into the output file. If a tab is read, the next position is calculated with tabstop.
 * @details this function reads each character from the input file until the end of file (EOF) occures. If it's a 'normal' character, the character is
 * directly written into the output file. If a tabstop occures, the position of the next character is calculated with p = (tabstop ((count / tapstop) + 1)), 
 * with count beeing the position of the character in the current line.
 * 
 * @param input_file - file to read from (file or stdin)
 * @param output_file - file to write in (file or stdout)
 * @param tabstop - number of tabstop, with which the position of the next character after a tab is calculated
 */
static void read_file(FILE *input_file, FILE *output_file, int tabstop) {
    char c = fgetc(input_file);
    int p;
    int count = 0;
    //while end of file is not reached
    while(c != EOF) {
        if(c == '\n') {
            //print new line and set count to 0
            fprintf(output_file, "\n");
            count = 0;
        } else if(c == '\t') {
            //calculate the position of the next character
            p = tabstop * ((count / tabstop) + 1);
            //print (p-count) spaces
            for(int i=0; i < (p-count); i++) {
                fprintf(output_file, "%c", ' ');
            }
            //add number of spaces printed to count
            count += (p-count);
        } else {
            //print character to output file and increase count
            fprintf(output_file, "%c", c);
            count++;
        }
        //read next character
        c = fgetc(input_file);
    }
    if(output_file == stdout) {
        fprintf(output_file, "\n");
    }
}


/**
 * main
 * @brief the entry point of the program
 * @details this function is the entry point of the program 'myexpand'. It handles the file checking, opening and closing. It also uses the other functions 
 * parse_argument, which parses the arguments of the prorgram (argv), and read_file, which reads from an input file and write into an output file while calculatng
 * position after a tab with tabstop. If the program could be run without causing an error, the program is exited with EXIT_SUCCESS. Otherwise
 * the program is terminated with EXIT_FAILURE.
 * 
 * @param argc - number of arguments
 * @param argv - value of the arguments
 * @return int - exit status (EXIT_FAILURE or EXIT_SUCCESS)
 */
int main(int argc, char *argv[]) {
    //parse arguments
    char *output_filepath = NULL;
    char *tabstop_c = NULL;
    int number_inputf = parse_arguments(argc, argv, &output_filepath, &tabstop_c);

    //get paths of input files
    char *input_filepaths[number_inputf];
    for(int i=0; i < number_inputf; i++) {
        input_filepaths[i] = argv[argc - number_inputf + i];
    }

    int tabstop = 8;
    char *succ;
    if(tabstop_c != NULL) {
        //convert to integer
        tabstop = strtol(tabstop_c, &succ, 10);
    } 

    //open outputfile if there is one -> else stdout
    FILE *output_file = stdout;
    if(output_filepath != NULL) {
        output_file = fopen(output_filepath, "w");
        if(output_file == NULL) {
            //output file couldn't be opened -> ERROR
            fprintf(stderr, "%s: ERROR: couldn't open file %s: %s\n", argv[0], output_filepath, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    //open & read inputfiles if there are any -> else stdin
    FILE *input_file = stdin;
    if(number_inputf != 0) {
        //read from input files
        for(int i=0; i < number_inputf; i++) {
            input_file = fopen(input_filepaths[i], "r");
            if(input_file == NULL) {
                //input file couldn't be opened -> ERROR
                fprintf(stderr, "%s: ERROR: couldn't open file %s: %s\n", argv[0], input_filepaths[i], strerror(errno));
                exit(EXIT_FAILURE);
            }
            read_file(input_file, output_file, tabstop);
            //close input file
            fclose(input_file);
        }
    } else {
        //read from sdtin
        read_file(stdin, output_file, tabstop);
    }

    if(output_filepath != NULL) {
        //close output file
        fclose(output_file);
    } 

    exit(EXIT_SUCCESS);
}