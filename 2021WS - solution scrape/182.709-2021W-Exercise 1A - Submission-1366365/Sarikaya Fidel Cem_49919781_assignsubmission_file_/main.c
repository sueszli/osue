/**     
*@file main.c
*@author Fidel Cem Sarikaya <e11941488@student.tuwien.ac.at>
*@date 08.11.2021
*
*@brief Main program module.
*
* This program receives a text input either from a file, whose name is given as an argument or from 'stdin' 
* and compresses it using run-length coding algorithm, then either writes the output to 'stdout' or to a file that it creates,
* whose name also given as an argument.
**/

#include "mycompress.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static char *MYPROG; /** The program name variable, whose value later to be taken from argv[] */

/**
 * Usage function.
 * @brief This function writes helpful usage information about how program should be called to stderr.
 * @details global variables: MYPROG;
 */
static void usage(void) {
    fprintf(stderr,"Usage Error! \tProper input: %s [-o output_file] [input_file]\n", MYPROG);
    exit(EXIT_FAILURE);
    }

/** 
 * Program entry point.
 * @brief The program starts here. According the arguments in 'argv' input and the output of the program are defined.
 * Afterwards input text gets compressed using the compression function and an output is given.
 * @details getopt() function parses the arguments, terminates the program with error messages in relevant cases and hands 
 * the name of the output file to the constant 'OUTPUTFILE_NAME', if given by the user. Character array 'in' is defined depending on input arguments. 
 * If an input file is given, 'in' receives lines from the file consecutively. If no file is given, 'in' takes user input from 'stdin'. 
 * In both cases, dynamic memory allocation is used, and afterwards relevant resources are freed.
 * The function then runs with paramters Character array 'in' and character array 'out'. If 'OUTPUTFILE_NAME' is null, it writes to 'stdout'. Else, creates the output file with it.
 * In the end dynamically allocated memory is freed.
 * global variables: MYPROG
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
*/
int main(int argc, char *const argv[])
{   
    MYPROG = argv[0];
    const char *OUTPUTFILE_NAME = NULL;

    int opt;
    while((opt = getopt(argc, argv, "o:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'o': 
                OUTPUTFILE_NAME = optarg;
                break; 
            case '?': 
                usage();
                fprintf(stderr, "Unknown Option: %c\n", optopt);
                break; 
            default:
                assert(0);
                break;
        } 
    } 
    
    char *in = NULL;
    if (argc > 4 || (argc - optind > 1) ) {usage();}
    else {
        if (argc - optind == 1) {
            const char *INPUTFILE_NAME = argv[optind];
            FILE *infile = fopen(INPUTFILE_NAME, "r");
            
            if (infile == NULL)
            {
            fprintf(stderr, "Input file not found.\n");
            return EXIT_FAILURE;
            }

            char *line = NULL;
            ssize_t read;
            size_t size = 0;
            
            int count = 0;
            while ((read = getline(&line, &size, infile)) != -1) {
                count++;
                if (count == 1) {
                    in = (char *)malloc(read);
                    strncpy(in, line, read);
                    in[read] = '\0';
                    continue;
                }
                char *tmp = realloc(in, strlen(in) + read + 1);
                if(!tmp) return EXIT_FAILURE;
                in = tmp;

                sprintf(in + strlen(in), "%s", line);
               }
            free(line);
            fclose(infile);
        }
        else {
            char *line = NULL;
            ssize_t read;
            size_t size = 0;

            printf("\nEnter a line: ");
            read = getline(&line, &size, stdin);

            in = (char *)malloc(read);
            strncpy(in, line, read-1);
            in[read-1] = '\0';
            
            free(line);
        }
    }

    char* out = (char*)malloc(sizeof(char) * (strlen(in) * 2 + 1));
    compressor(in, out);
    
    if (!OUTPUTFILE_NAME) {
        fprintf(stdout, "\n%s\n", out);
    }
    else {
        FILE *outfile = fopen(OUTPUTFILE_NAME, "w");
        fputs(out, outfile);
        fclose(outfile);

        fprintf(stdout, "\n***\nFile %s is created.\n***", OUTPUTFILE_NAME);
    }
    fflush(stdout);
 
    fprintf(stderr, "\n\nRead: \t\t\t%d characters\nWritten: \t\t%d characters\nCompression ratio: \t%0.1f%%\n\n", 
                    (int)strlen(in), (int)strlen(out), (((float)strlen(out) / strlen(in)) * 100));

    free(out);
    free(in);
    
    return EXIT_SUCCESS;
}
