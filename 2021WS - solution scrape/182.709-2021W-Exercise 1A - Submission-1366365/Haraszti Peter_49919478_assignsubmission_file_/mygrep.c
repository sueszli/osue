/**
 * @file mygrep.c
 * @author Peter Haraszti <Matriculation Number: 12019844>
 * @date 03.11.2021
 *
 * @brief mygrep - a simplified version of the UNIX grep command
 *
 * @details mygrep works similarly to the UNIX grep command. It takes a keyword and some text as an input and returns the lines, in which the keyword is contained.
 * The text to be grepped from can either be entered manually, or be read from one or multiple files.
 * The option -i determines, if mygrep is case sensitive or not.
 * The option -o can specify a file, to which the output is written.
 * Usage: mygrep [-i] [-o outfile] keyword [file...]
 **/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>


#define CASE_SENSITIVE 0
#define NOT_CASE_SENSITIVE 1

char *userInput;

//! Contains the name of the program obtained from argv[0]
char *programname = NULL;

/**
 * @brief Prints the usage message
 * @details This function should be called if the user tries to use the program in an unforseen way. Exact instructions on how to use the program are printed.
 * @param void
 * @return void
 */
void usage(void) {
    fprintf(stderr, "Usage: mygrep [-i] [-o outfile] keyword [file...]\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief Writes the contents of *text into *outfile in the specified mode *mode
 * @details The file *outfile is opened, if that fails, the program exits with an error. Otherwise the given text is written into the file specified in *outfile
 * @param text Pointer to char[], in which the text to be written is contained
 * @param outfile Filename of the file in which the given text is written
 * @param mode Mode to open the file, e.g "w", "a", "a+", ...
 * @return void
 */
void writeToFile(char *text, char *outfile, char *mode) {
    FILE *fp;
    fp = fopen(outfile, mode);
    if (fp == NULL) { // In case opening the file fails
        fprintf(stderr, "%s: Can't open given file\n", programname);
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "%s", text);
    fclose(fp);
}

/**
 * @brief Greps the keyword from stdin
 * @details If the input from stdin contains the keyword passed as a parameter, the line is either written to stdout or into *outfile if *outfile is not NULL.
 * If the parameter caseSensitive is 1, both the user input and the keyword are converted to lowercase so that the grep is not case sensitive.
 * @param keyword Keyword to be grepped
 * @param caseSensitive if the value is 1, grepFromStdin is not case sensitive
 * @param outfile file in which the result of grepFromStdin will be stored. If outfile is NULL, the result is printed to stdout
 * @return void
 */
void grepFromStdin(char *keyword, int caseSensitive, char *outfile) {
    // Read user input from stdin
    size_t bufsize = 32;
    size_t characters;
    userInput = (char *) malloc(bufsize * sizeof(char)); //dynamically allocated memory where the input is stored
    if (userInput == NULL) {
        fprintf(stderr, "%s: Unable to allocate userInput\n", programname);
        exit(EXIT_FAILURE);
    }
    while((characters = getline(&userInput, &bufsize, stdin)) != -1){; // Number of characters read

        // Save the original input in case it is modified later on
        char userInputOriginal[characters+1];
        strcpy(userInputOriginal, userInput);

        // convert keyword and input to lowercase if -i option was given
        if (caseSensitive == NOT_CASE_SENSITIVE) {
            int i = 0;
            while (userInput[i]) {
                userInput[i] = tolower(userInput[i]);
                i++;
            }

            i = 0;
            while (keyword[i]) {
                keyword[i] = tolower(keyword[i]);
                i++;
            }
        }

        // Check if the keyword is a substring of userInput. If not, sub is NULL
        char *sub = strstr(userInput, keyword);

        // Print or write the result
        if (outfile == NULL) { // In case there was no file given to write to, print to stdout
            if (sub != NULL) {
                printf("%s", userInputOriginal);
            }
        } else {
            if (sub != NULL) { //If the input contained the keyword, write it to the file, otherwise clear the file
                writeToFile(userInputOriginal, outfile, "a");
            } else {
                writeToFile("", outfile, "a");
            }
        }

    }

    // Free allocated memory
    if (userInput) {
        free(userInput);
    }
}

/**
 * @brief Greps the keyword from a specified file
 * @details grepFromFile attempts to open *file for reading. If that fails, the program exits with an error.
 * Otherwise the file is read line by line. If a line contains the string *keyword, that line is either printed to stdout if the given *outfile is NULL or written into *outfile otherwise.
 * If the parameter caseSensitive is 1, both the line from the file and the keyword are converted to lowercase so that the grep is not case sensitive.
 * @param keyword Keyword to be grepped
 * @param caseSensitive if the value is 1, grepFromStdin is not case sensitive
 * @param outfile file in which the result of grepFromFile will be stored. If outfile is NULL, the result is printed to stdout
 * @param file file from which the lines to be grepped are read
 * @return void
 */
void grepFromFile(char *keyword, int caseSensitive, char *outfile, char *file) {
    if (caseSensitive == NOT_CASE_SENSITIVE) { // Make keyword lowercase if -i parameter was passed
        int i = 0;
        while (keyword[i]) {
            keyword[i] = tolower(keyword[i]);
            i++;
        }
    }

    // Create dynamic buffer for the file and attempt to open it
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(file, "r");
    if (fp == NULL) { // In case reading the file fails
        fprintf(stderr, "%s: Can't open given file\n", programname);
        exit(EXIT_FAILURE);
    }

    // Read lines of the file while there are any
    while ((read = getline(&line, &len, fp)) != -1) {
        char originalLine[read+1];
        strcpy(originalLine, line); // Save the original line in case it is modified later on

        if (caseSensitive == NOT_CASE_SENSITIVE) { // Convert the line to lowercase if -i parameter was passed
            int i = 0;
            while (line[i]) {
                line[i] = tolower(line[i]);
                i++;
            }
        }

        // Check if the keyword is a substring of the current line. If not, sub is NULL
        char *sub = strstr(line, keyword);

        // Print or write the result
        if (outfile == NULL) { // In case there was no file given to write to, print to stdout
            if (sub != NULL) {
                printf("%s", originalLine);
            }
        } else {
            if (sub != NULL) { //If the input contained the keyword, write it to the file, otherwise append nothing
                writeToFile(originalLine, outfile, "a");
            } else {
                writeToFile("", outfile, "a");
            }
        }
    }

    // Close the file and free allocated memory
    fclose(fp);
    if (line) {
        free(line);
    }
}

/**
 * @brief Handling of SIGINT and SIGTERM
 * @details In case a signal SIGINT or SIGTERM occurs, resources are cleaned up.
 * @param signal
 * @return void
 */
void handle_signal(int signal) {
    if (userInput) {
        free(userInput);
    }
    _exit(1);
}

/**
 * @brief Sets up Signal Handling
 * @details If a SIGTERM or a SIGINT occurs, handle_signal is called
 * @param void
 * @return void
 */
void initSignalHandling(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/**
 * @brief Main function of mygrep, a simplified version of the UNIX grep command
 * @details First, the command line options are with getopt().
 * The possible parameters are: -i for telling mygrep that the grep should not be case sensitive; -o for specifying a file to which the result should be written to.
 * If the only argument besides the options is the keyword, then the function getFromStdin() is called, which greps the keyword from the user input (stdin).
 * If there are more arguments, the other arguments are interpreted as files, that contain lines to be grepped. In that case for each given file the function grepFromFile() is called.
 * If the user tries to use mygrep in an unforseen way, the function usage() is called, which displays a usage message.
 * If no errors occur while running mygrep, the program exits with EXIT_SUCCESS.
 * @param argc number of arguments
 * @param argv array of arguments
 * @return EXIT_SUCCESS, if the program doesn't crash before reaching this line
 */

int main(int argc, char **argv) {

    initSignalHandling();

    int opt_i = 0;
    char *o_arg = NULL;
    int c;

    // Get options from command line. -o takes a parameter o_arg
    while ((c = getopt(argc, argv, "i o:")) != -1) {
        switch (c) {
            case 'i':
                opt_i++;
                break;
            case 'o':
                o_arg = optarg;
                break;
            default:
                break;
        }
    }

    programname = argv[0];

    if (argc > optind) { // If there is at least a keyword in the command line arguments
        char *keyword = NULL;
        keyword = argv[optind];

        if (argc == optind + 1) { // Zero files given, read from keyboard input
            if (o_arg != NULL) { // In case an output file is given, clear it before appending the new grep results
                writeToFile("", o_arg, "w");
            }
            grepFromStdin(keyword, opt_i, o_arg);
        }
        if (argc > optind + 1) { // There is at least one file as a parameter
            if (o_arg != NULL) { // In case an output file is given, clear it before appending the new grep results
                writeToFile("", o_arg, "w");
            }
            for (int i = optind + 1; i < argc; i++) { // Grep from each file given
                char *file = NULL;
                file = argv[i];
                grepFromFile(keyword, opt_i, o_arg, file);
            }
        }
    } else { // If the number of the command line arguments is insufficient
        usage();
    }

    return EXIT_SUCCESS;
}
