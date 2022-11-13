/**
 * @file ispalindrom.c
 * @author Christoph Weber 11908087 <e11908087@student.tuwien.ac.at>
 * @date 12 Nov 2021
 * @brief Program which checks if textlines are palindroms.
 *
 * The program reads files line by line and for each line checks whether it is a palindrom,
 * i.e. whether the text read backwards is identical to itself. Each line is printed followed by the text
 * "is a palindrom" if the line is a palindrom or "is not a palindrom" if the line is a not palindrom.
 *
 * If the option -o is given, the output is written to the specified file (outfile). Otherwise, the output is
 * written to stdout.
 *
 * The option -s causes the program to ignore whitespaces when checking whether a line is a palindrom. If
 * the option -i is given, the program does not differentiate between lower and upper case letters, i.e. the
 * check for a palindrom is case insensitive
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define warning() fprintf(stderr, "Error in: %s: ", MyProg); /**< macro used to print the the filename before each stderr output*/

const char *MyProg; /**< The program name.*/

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stdout.
 * @details global variables: MyProg
 */
static void usage(void) {
    fprintf(stdout, "Usage: %s [-s] [-i] [-o file] file\n", MyProg);
    exit(EXIT_FAILURE);
}


/**
 * @brief Reads an input file checks it for palindromes on line basis and then writes the resuslts to the output file
 * @details Both input and output have to be valid files. Will copy the text provided
 * in the input file to the output file and proceeds each line with either is a palindrom
 * or is not a palindrom. Can ignore case or whitespaces which can be enabled via the parameters.
 * global variables: MyProg
 * @param ingnoreSpace defines if spaces should be ignore.
 * @param caseInsensetive defines the palindrom should be checked case insensetive.
 */
static void findPali(FILE *input, FILE *output, int ignoreSpace, int caseInsensetive) {
    char *line;
    size_t lineSize = 64;
    line = malloc(sizeof(char) * lineSize);
    int done = 0;
    int isPali;
    char *response;
    ssize_t read;
    while (!done) {
        isPali = 1;
        read = getline(&line, &lineSize, input);
        if (-1 == read) {
            break;
        } else if (line[0] == '\n') {
            continue;
        }
        //lowering read by one because its used as an index and not as a size factor
        --read;
        //removing newline if present for the output format
        if (line[read] == '\n') {
            line[read] = '\0';
            --read;
        }
        //checks both sides of the string if they are mirrored
        for (ssize_t i = 0, j = read; i < j;) {
            if (ignoreSpace) {
                if (line[i] == ' ') {
                    i++;
                    continue;
                } else if (line[j] == ' ') {
                    j--;
                    continue;
                }
            }
            if (caseInsensetive) {
                if (tolower(line[i]) != tolower(line[j])) {
                    isPali = 0;
                    break;
                }
            } else if (line[i] != line[j]) {
                isPali = 0;
                break;
            }
            i++;
            j--;
        }

        if (isPali) {
            response = " is a palindrom";
        } else { response = " is not a palindrom"; }
        if (fprintf(output, "%s%s\n", line, response) < 2) {
            warning()
            fprintf(stderr, "Couldn't write to output file\n");
            if (input != stdin) fclose(input);
            fclose(output);
            free(line);
            exit(EXIT_FAILURE);
        }
    }
    fflush(output); //to ensure everything is written to the output
    free(line);
}


/**
 * Program entry point.
 * @brief Takes file location for input and output and then checks the input files for palindroms
 * @details Checks if input and output files are given. If yes then it makes sure they are valid files.
 * otherwise stdin and stdout are used as defaults. 
 * global variables: MyProg
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE if a failure has ocurred.
 */
int main(int argc, char **argv) {

    FILE *input = stdin, *output = stdout; //default if no arguments are given
    MyProg = argv[0];

    int c;
    int opt_o = 0;
    int ignoreSpace = 0;
    int caseInsensetive = 0;

    while ((c = getopt(argc, argv, "sio:")) != -1) {
        switch (c) {
            case 'o':
                ++opt_o;
                if (opt_o > 1) //only one output file should be called, otherwise the usage is printed
                {
                    if (output != NULL && output != stdout) fclose(output);
                    usage();
                }
                output = fopen(optarg, "w");
                break;
            case 's':
                ignoreSpace = 1;
                break;
            case 'i':
                caseInsensetive = 1;
                break;
            default:
                usage();
        }

    }

    //if no arguments where passed along the call of the method
    if (output == NULL) {
        warning()
        fprintf(stderr, "Couldn't open output file\n");
        exit(EXIT_FAILURE);
    }

    if (optind == argc) { //no arguments for the input file where passed therefore the standard stdin is used
        findPali(input, output, ignoreSpace, caseInsensetive);
    }
    for (; optind < argc; optind++) { //runs when extra arguments/files are passed and reads all files
        input = fopen(argv[optind], "r");
        if (input == NULL) {
            warning()
            fprintf(stderr, "couldn't open file: %s\n", argv[optind]);
        } else {
            //calls palindrom finder on the input file
            findPali(input, output, ignoreSpace, caseInsensetive);
            if (fclose(input) < 0) {
                warning()
                fprintf(stderr, "couldn't close file: %s\n", argv[optind]);
                exit(EXIT_FAILURE);
            }
        }
    }
    if (output != stdout)
        fclose(output);
    //input doesnt has to be closed since files are handled in the function above and stdin doesn't need closing

    exit(EXIT_SUCCESS);
}