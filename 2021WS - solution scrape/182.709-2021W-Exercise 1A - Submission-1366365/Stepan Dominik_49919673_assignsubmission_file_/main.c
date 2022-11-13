/**
 * @file main.c
 * @author Dominik Stepan (01327601)
 * @brief Reads in two files and compares them. 
 * If two lines differ, then the line number and the number of differing characters is printed.
 * @date 2021-11-13
 * 
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHAR_LIMIT (100)
const char *PROGRAM_NAME = NULL;

/**
 * @brief 
 * Handles each line of both files and compares them 
 * @param file1 file1 
 * @param file2 file2 
 * @param outfile_name Optional outfile name
 * @param i_flag if true is case insensitive
 */
static void myDiff(FILE *file1, FILE *file2, char *outfile_name, int i_flag);

/**
 * @brief Reads in two files and compares them. 
 * If two lines differ, then the line number and the number of differing characters is printed.
 * @param argc argument counter
 * @param argv argument vector
 * @return int return 0 on success
 */
int main(int argc, char *argv[]) {
    int opt;
    char *outfile_name = NULL;
    PROGRAM_NAME = argv[0];
    int i_flag = 0;

    /*Get Options*/
    while ((opt = getopt(argc, argv, "io:")) != -1) {
        switch (opt) {
            case 'i':
                i_flag = 1;
                break;
            case 'o':
                outfile_name = optarg;
                break;
            case '?':
                return 1;
            default:
                assert(0);  //defensive programming
        }
    }

    // Check if 2 files were given
    if ((((argc - i_flag) != 3) && (outfile_name == NULL)) || (((argc - i_flag) != 5) && (outfile_name != NULL))) {
        (void)fprintf(stderr, "%s: Wrong arguments, use ./mydiff [-i] [-o outfile] file1 file2 \n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    //Open both files
    FILE *in1 = fopen(argv[argc - 2], "r");
    FILE *in2 = fopen(argv[argc - 1], "r");

    if (in1 == NULL) {
        (void)fprintf(stderr, "%s: Error opening the 1st file. \n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }
    if (in2 == NULL) {
        (void)fprintf(stderr, "%s: Error opening the 2nd file. \n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    (void)myDiff(in1, in2, outfile_name, i_flag);

    //Close
    (void)fclose(in1);
    (void)fclose(in2);

    return EXIT_SUCCESS;
}

static void myDiff(FILE *file1, FILE *file2, char *outfile_name, int i_flag) {
    FILE *outfile;

    //Open outfile_name, if given
    if (outfile_name != NULL) {
        outfile = fopen(outfile_name, "a+");
        if (outfile == NULL) {
            (void)fprintf(stderr, "%s: Error while opening the file. \n", PROGRAM_NAME);
            exit(EXIT_FAILURE);
        }
    }

    char *line1 = malloc(CHAR_LIMIT);
    char *line2 = malloc(CHAR_LIMIT);
    int linecount = 0;
    int differences = 0;

    //Read each line until one of the input files ends
    while (fgets(line1, CHAR_LIMIT, file1) && fgets(line2, CHAR_LIMIT, file2)) {
        int newline_found = 0;
        for (int i = 0; line1[i] != '\0' && line2[i] != '\0'; i++) {
            char c1 = line1[i];
            char c2 = line2[i];
            //convert to lower if i flag is given
            if (i_flag) {
                c1 = tolower(c1);
                c2 = tolower(c2);
            }
            if (c1 == '\n' || c2 == '\n') {
                //increase line count (necessary if size is longer than the buffer)
                linecount++;
                //only report results once newline is found to prevent multiple reports/line because buffer is full
                newline_found = 1;
            } else if (c1 != c2) {
                //count diff
                differences++;
            }
        }

        //Write to stdout or file
        if (differences > 0 && newline_found) {
            if (outfile_name == NULL) {
                (void)printf("Line: %d, characters: %d\n", linecount, differences);
            } else {
                (void)fprintf(outfile, "Line: %d, characters: %d\n", linecount, differences);
            }
            differences = 0;
        }
    }
    //cleanup
    free(line1);
    free(line2);

    if (outfile_name != NULL) {
        (void)fclose(outfile);
    }
}