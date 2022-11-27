/**
 * @file myexpand.c
 * @author Elias Pinter <e12023962@student.tuwien.ac.at>
 * @date 23.10.2021
 * @brief Program myexpand: This program searches for tab symbols in an input file and replaces it with spaces.
 * @details If there are input files passed as an argument the program reads from those filesm 
 * otherwise it reads from stdin. If there is an output file passed as an argument, it writes to this file, else
 * it writes to stdout. The program reads until it finds a '\t' character and replaces it with a specific amount of
 * spaces depending on the position, where the tab-character was found. 
 **/


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 80

static char* pgm_name;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name: the program name
 */
static void usage(void){
    (void)fprintf(stderr, "USAGE: %s [-t tabstop] [-o outfile] [file...]\n", pgm_name);
    exit(EXIT_FAILURE);
}



/**
 * @brief This function replaces the tabs of the input file with the specified number spaces and 
 * writes to the output file given with -o or to stdout otherwise. 
 * @details Reading and writing is performed with the help of a buffer. Its size is defined by the Macro BUFFER_SIZE
 * 
 * @param input: is a FILE pointer to an input file or stdin from which the function reads.
 * @param tabstop: integer which is used to calculate the number of space characters, that must be inserted.
 * @param output: is a FILE pointer to an ouput file or stdout to which the function writes
 */
static void writeFile(FILE* input, int tabstop, FILE* output){

    char buffer[BUFFER_SIZE];

    // position to start writing again after inserting spaces
    int p;

    int numOfSpaces;

    // keeps track of the position in the line currently read
    int x = 0;

    // reads the input file line for line into a buffer
    while (fgets(buffer, BUFFER_SIZE, input) != NULL) {

        // then the program searches for tab-characters in the buffer  
        for (int k = 0; k < strlen(buffer); k++) {

            // if a tab-character is found the number of spaces that need to be written to the output file
            // is calculated
            // if any other character is read it is just written to the output file.
            if (buffer[k] == '\t') {
                p = tabstop * ((x / tabstop) + 1);
                numOfSpaces = p - x;

                // the program tries to write the specified amount of spaces to the output file
                while (numOfSpaces > 0) {
                    int fptc = fputc(' ', output);

                    if (fptc == EOF) {
                        fprintf(stderr, "%s : %s \n", pgm_name, strerror(errno));
                        fclose(input);
                        fclose(output);
                        exit(EXIT_FAILURE);
                    }

                    numOfSpaces--;

                    // position inside the line increases
                    x++;
                }
            }
            else {

                // tries to write the read character to the output file
                int fptc = fputc(buffer[k], output);

                if (fptc == EOF) {
                    fprintf(stderr, "%s : %s \n", pgm_name, strerror(errno));
                    fclose(input);
                    fclose(output);
                    exit(EXIT_FAILURE);
                }

                // position inside the line increases
                x++;
            }
            if (buffer[k] == '\n') {

                // position is reset if a new line starts
                x = 0;
            }
        }
    }

}
/**
 * Program entry point.
 * @brief The program starts here and will take the options and arguments from the console.
 * @details Option -t tabstop is an integer which is used to calculate the number of spaces to replace the
 * found tab-characters with. if the option isn't given tabstop is set to 8.
 * Option -o output sets the output locatio to the file outout. If the option isn't given ouput
 * is set to stdout. 
 * global variables: pgm_name: the program name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCESS on normal termination and EXIT_FAILURE otherwise.
 */
int main(int argc, char** argv){
    pgm_name = argv[0];
    char* arg_o = NULL;
    char* arg_t = NULL;
    int c;

    // reads in the option passed as argument
    while ((c = getopt(argc, argv, "t:o:")) != -1) {
        switch (c) {
        case 't':
            arg_t = optarg;
            break;

        case 'o':
            arg_o = optarg;
            break;

        default:
            usage();
        }
    }

    int tabstop;
    char* endptr;

    // if tabstop is passed as an argument it is read and pared to a long integer
    // if the option -t isnt given tabstop is set to 8
    if (arg_t != NULL) {
        tabstop = strtol(arg_t, &endptr, 10);
        if(tabstop < 1){
            fprintf(stderr, "%s : invalid input \n", argv[0]);
            return EXIT_FAILURE;
        }

        if (*endptr) {
            fprintf(stderr, "%s : invalid input \n", argv[0]);
            return EXIT_FAILURE;
        }

    } else {
        tabstop = 8;
    }

    FILE* output = stdout;

    // if the option -o is given the output file passed as an argument is opened
    // if the option is not given the output will be written to stdout
    if (arg_o != NULL) {
        output = fopen(arg_o, "w");

        if (output == NULL){
            fprintf(stderr, "%s : %s \n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }

    }
    // if there are no input files passed as arguments the input will be read from stdin
    if (optind == argc) {
        FILE* input = stdin;
        writeFile(input, tabstop, output);        
    }

    // if there are input files passed as arguments then they will be tried to open one after another 
    for (int j = optind; j < argc; j++) {
        FILE* input = fopen(argv[j], "r");

        if (input == NULL) {
            fprintf(stderr, "%s : %s \n", argv[0], strerror(errno));
            fclose(output);
            return EXIT_FAILURE;
        }

        // after the input file is opened writeFile will start to write the output to the output file.
        writeFile(input, tabstop, output);

        // after the output is written to the output file the current input file is tried to be closed
        int fc = fclose(input);

        if(fc == EOF){
            fprintf(stderr, "%s : %s \n", argv[0], strerror(errno));
            fclose(output);
            return EXIT_FAILURE;
        }
    }

    // after all input files are done the ouput file is tried to be closed
    int fc = fclose(output);

     if(fc == EOF){
         fprintf(stderr, "%s : %s \n", argv[0], strerror(errno));
         return EXIT_FAILURE;
        }
    return EXIT_SUCCESS;
}