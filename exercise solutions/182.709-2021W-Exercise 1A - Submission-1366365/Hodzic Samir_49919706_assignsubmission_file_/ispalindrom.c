/**
 * @file ispalindrom.c
 * @author Samir Hodzic <e12012407@student.tuwien.ac.at>
 * @date 14.11.2021
 * 
 * @brief This program reads files line by line and checks whether it is a palindrom.
 * 
 * @details The program takes a file and checks each line if it is a palindrom, i.e. whether the text read backwards is identical to itself.
 * If a line is a palindrom, the program will print the word followed by the text "is a palindrom", otherwise "is not a palindrom".
 * 
 * If sepcified with the [-o outfile] option, the output is written to the specified file.
 * Otherwise it is written to stdout.
 * 
 * Given the -s option the program will ignore whitespaces when checking if the line is a palindrom.
 * Given the -i option the program will not differentiate between lower and upper case letters, i.e. it removes the case sensitivity.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>

/**
 * @brief This struct is used to manage the given options and arguments
 * @details default options are set in main, be sure to overwrite them
 */
typedef struct options {
    bool s_given;
    bool i_given;
    bool is_stdin;
    char *output;
    FILE *output_file;
} options_t;

/**
 * @brief *name is the name of the program and unchangeable.
 * @details *name is static to ensure that there will be no modifications.
 */
static char *name = "ispalindrom";

/**
 * @brief Prints program usage.
 * @details Exits the program afterwards.
 */
static void usage(void) {
    fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints error message.
 * @details Uses usage() at the end to print the correct usage and exit the program correctly.
 */
static void error_message(char *err_msg) {
    fprintf(stderr, "%s ERROR: ", name);
    fprintf(stderr, "%s", err_msg);
    fprintf(stderr, "\n");
    usage();
}

/**
 * @brief Checks if the given string is a palindrom.
 * @details If the given string is a palindrom, the function returns the boolean "true".
 * If not, the function will return "false".
 */
bool is_palindrom(char str[]) {
    int length = strlen(str);

    for (int i = 0; i < length; i++) {
        if (str[i] != str[length-1-i]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Ignores the whitespace in a given string.
 * @details The function removes all the whitespaces of the given input resulting in a shorter string.
 */
void trim_whitespace(char str[]) {
    int length = strlen(str);
    char temp[length];

    for (int i = 0; i < length; i++) {
        temp[i] = str[i];
    }
    
    int i = 0;
    int j = 0;
    while (i < length) {
        if (temp[i] != ' ') {
            str[j] = temp[i];
            j++;
        }
        i++;
    }
    str[j] = '\0';
}

/**
 * @brief Converts the given string to the lowercase version.
 * @details Every uppercase letter of the string becomes the lowercase one using the function tolower().
 */
void lower_case(char word[]) {
    int i = 0;
    while (word[i] != '\0') {
        // cast because tolower() returns int
        word[i] = (char) tolower(word[i]);
        i++;
    }
}

/**
 * @brief Checks one single line.
 * @details If the given string is NULL the function will return an error message.
 * If not, check_line checks which options are activated and calls the right functions up.
 * Then it checks whether the given string is a palindrom or not.
 * If so, the function prints "... is a palindrom" and if not, "... is not a palindrom".
 */
static void check_line(char *str, options_t *options) {
    if (str == NULL) {
        error_message("given string is NULL");
    }

    // terminate string where the newline char is
    str[strcspn(str, "\n")] = '\0';

    // copy string to use the options on it
    int length = strlen(str);
    char temp[length];
    strcpy(temp, str);

    if (options->s_given) {
        trim_whitespace(temp);
    }
    if (options->i_given) {
        lower_case(temp);
    }

    if (is_palindrom(temp)) {
        fprintf(options->output_file, "%s is a palindrom\n", str);
    } else {
        fprintf(options->output_file, "%s is not a palindrom\n", str);
    }
}

/**
 * @brief Handles each line of the given file and checks if it is a palindrom.
 * @details The function handles all lines of the input file and calls the check_line function up to check it.
 * If the length of the string is 0, the program skips the check_line call-up to avoid errors.
 */
static void check_file(FILE *file, options_t *options) {
    char *str = NULL;
    size_t size = 0;

    while (getline(&str, &size, file) != -1) {
        if (strlen(str) == 0) {
            continue;
        }

        check_line(str, options);
    }

    // str needs to be freed up even if getline() fails
    free(str);
}

/**
 * @brief Handles arguments from the command line.
 * @details The given options [-s], [-i] and [-o outfile] are defined with getopt() so that options can be activated in the command line.
 * 
 * @param argc This is the number of the arguments and is from main().
 * @param argv The values of the arguments are stored here and it is also from main().
 * @param options These are the options that are being set with this function.
 */
static int handle_args(int argc, char **argv, options_t *options) {
    int c;
    name = argv[0];

    while((c = getopt(argc, argv, "sio:")) != -1) {
        switch (c) {
            case 's':
                options->s_given = true;
                break;
            case 'i':
                options->i_given = true;
                break;
            case 'o':
                // store pointer of the text after [-o] to output from options struct
                options->output = optarg;
                break;
            default:
                usage();
        }
    }
    // index of the next element to be processed in argv
    return optind;
}

/**
 * @brief This is the main function.
 * @details The default options of the options struct are set here.
 * 
 * @param argc Number of arguments.
 * @param argv Values of the arguments.
 * @return
 */
int main(int argc, char **argv) {
    options_t options;
    options.s_given = false;
    options.i_given = false;
    options.is_stdin = false;
    options.output = NULL;

    // set is_stdin to true if there are no input files
    int pos = handle_args(argc, argv, &options);
    if ((argv[pos] == NULL) || (pos >= argc)) {
        options.is_stdin = true;
    }

    FILE *file;
    if (options.output == NULL) {
        file = stdout;
    } else {
        file = fopen(options.output, "w");
        if (file == NULL) {
            error_message("File does not exist");
        }
    }
    options.output_file = file;

    if (options.is_stdin) {
        check_file(stdin, &options);
    } else {
        while (argv[pos] != NULL) {
            FILE *read;
            read = fopen(argv[pos++], "r");
            if (read == NULL) {
                error_message("File does not exist");
            }

            check_file(read, &options);
            fclose(read);
        }
    }

    if (options.output != NULL) {
        fclose(file);
    }
    
    return EXIT_SUCCESS;
}