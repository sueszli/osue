/**
 * @file main.c
 * @author Maximilian Podpera <e11825326@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Main program module.
 *
 * Program to check whether some input contains palindromes.
 **/
 #include <stdio.h> // Default
#include <unistd.h>// args
#include <assert.h> // switch
#include "stdlib.h" // Exit codes and free
#include <string.h> // String operations
#include <ctype.h>
#include <xlocale.h>

/**
 * @brief prints usage information to stderr
 * @params *name of the program.
 * @return void
 * @details prints usage information for the program. name argument will be prepended.
 **/
static void usage(char *name) {
    char * usageString =
            " [-s] [-i] [-o outfile] [file...]";
    printf("%s %s", name, usageString);
}

/**
 * @brief reverses a given char array
 * @params line the char array to reverse
 * @params start the index to start reversing the array
 * @params start the index to end reversing the array
 * @return void
 * @details reverse a given char array from a given index to a given end index. no bounds checking is performed.
 **/
static void reverse(char *line, int start, int end) {
    // break
    if (start > end) return;

    // swap
    char c;
    c = *(line + start);
    *(line + start) = *(line + end);
    *(line + end) = c;
    reverse(line, ++start, --end);
}

/**
 * @brief entry point for checking strings if they are palindromes
 * @details Input can be given as one ore more file. If none are given the stdin stream is used. Output will either be writen
 * to stdout or a file if one is given with the '-o' flag. An '-i' flag can be used to make the check case insensitive.
 * The '-s' flag makes the check ignore whitespaces.
 * @param argc number of arguments
 * @param argv list of arguments
 * @return EXIT_SUCCESS on success or EXIT_FAILURE on failure during the execution.
 */
int main(int argc, char **argv) {
    int iFlag = 0; // is the -i flag set (case insensitive)
    int sFlag = 0; // is the -s flag set (ignore whitespaces)
    char* outfile = NULL; // outfile if given

    int c; // option enumerating char
    // Get the set flags
    while ((c = getopt(argc, argv, "iso:")) != -1) {
        switch (c) {
            case 'i':
                iFlag = 1;
                break;
            case 's':
                sFlag = 1;
                break;
            case 'o':
                outfile = optarg;
                break;
            default:
                usage(argv[0]);
                assert(0);
        }
    }

    // +1 for each flag. (get index to position of positional arguments)
    int index = 1 + iFlag + sFlag + (outfile? 2:0);
    int fromStdIn = index >= argc; // are there arguments left?

    // Variables for reading from file
    char *nextLine = NULL;
    size_t nextLineLength = 0;
    FILE *fp;

    char *nextLineRev;
    char *nextLineWork;

    FILE *fpout;

    // Try to open the file
    if (outfile != NULL) {
        if ((fpout = fopen(outfile, "w")) == NULL) {
            fprintf(stderr, "%s:Could not open output file\n", argv[0]);
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    while (index < argc || fromStdIn) { // as long as more files are specified or input is read from stdin
        fp = stdin;
        if (!fromStdIn) fp = fopen(argv[index], "r");

        // Was the file opened?
        if (fp == NULL) {
            fprintf(stderr, "%s: Could not open file\n", argv[0]);
            usage(argv[0]);
            return EXIT_FAILURE;
        }

        // Actually reading the line
        while ((getline(&nextLine, &nextLineLength, fp)) != -1) {
            if (nextLine[strlen(nextLine)-1] == '\n') nextLine[strlen(nextLine) -1] = '\0'; // remove trailing newline
            nextLineWork = malloc(strlen(nextLine) + 1);

            // Handle the flags
            int i = 0, j = 0;
            while (nextLine[i]) {
                if (sFlag && nextLine[i] == ' ') {i++;}
                if (iFlag) {nextLineWork[j++] = toupper(nextLine[i]);}
                else {nextLineWork[j++] = nextLine[i];}

                i++;
            }

            // reverse
            nextLineRev = malloc(strlen(nextLineWork) + 1);
            strcpy(nextLineRev, nextLineWork);
            reverse(nextLineRev, 0, strlen(nextLineRev) -1);


            // compare
            if (strcmp(nextLineWork, nextLineRev) != 0){
                if (outfile != NULL) {
                    fprintf(fpout, "%s is not a palindrome\n", nextLine);
                } else {
                    printf("%s is not a palindrome\n", nextLine);
                }
            } else {
                if (outfile != NULL) {
                    fprintf(fpout, "%s is a palindrome\n", nextLine);
                } else {
                    printf("%s is a palindrome\n", nextLine);
                }
            }
            fflush(fpout);
        }
        //free(nextLine);
        fclose(fp);
        index++;
    }
    fclose(fpout);

    return EXIT_SUCCESS;
}

