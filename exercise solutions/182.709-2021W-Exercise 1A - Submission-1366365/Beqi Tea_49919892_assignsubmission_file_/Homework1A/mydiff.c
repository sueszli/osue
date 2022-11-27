/**
 * @file mydiff.c
 * @author Tea Beqi <e11904650@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief Main program module.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <strings.h>


static void closeFile(FILE * fPointer, char* fName);

static void calculateDifference(int cmp, FILE *fp1, FILE *fp2, FILE *fp3);

/**
 * Program entry point.
 * @brief The main program function
 *
 * @details This function takes care about parameters that could be passed on to main. In
 * this program we get the content of two text files and compares each line, based  on how
 * many characters differ in each line. This program can also save the output in a new file
 * that is also accessible.
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS. (if the files open correctly, otherwise the program exits
 * with EXIT_FAILURE)
 */

int main(int argc, char **argv)
{
    int cmp = 0;
    char* outFileName = NULL;
    int opt;
    FILE *fPointer1, *fPointer2, *fPointer3;

    while((opt = getopt(argc, argv, "io:")) != -1) {
        switch(opt) {
            case 'i':
                cmp = 1;
                break;
            case 'o':
                outFileName = optarg; // Using output file since -o was defined
                break;
            default:
                fprintf(stderr, "Error something wrong with the input parameters\n"); // Print error message
                exit(EXIT_FAILURE); // Exit main
        }
    }

    // Making sure that there are exactly two more arguments
    if((argc-optind) != 2){
        fprintf(stderr, "Error The number of input files is not 2\n");
        exit(EXIT_FAILURE);
    }

    fPointer1 = fopen(argv[optind], "r");
    fPointer2 = fopen(argv[optind + 1], "r");
    fPointer3 = NULL;


    // Making sure that the other two arguments are files and that they can be opened
    if(fPointer1 == NULL) {
        fprintf(stderr, "Error opening first file\n");
        exit(EXIT_FAILURE);
    }
    if(fPointer2 == NULL) {
        fprintf(stderr, "Error opening second file\n");
        exit(EXIT_FAILURE);
    }

    if(outFileName != NULL){
        fPointer3 = fopen(outFileName, "w");
        if(fPointer3 == NULL) {
            fprintf(stderr, "Error opening output file\n");
            exit(EXIT_FAILURE);
        }
    }

    // Here is when the comparing is being done
    calculateDifference(cmp, fPointer1, fPointer2, fPointer3);

    // Close the output file, if it was given in the arguments
    if(outFileName != NULL) {
        closeFile(fPointer3, outFileName);
    }

    // Close the input files
    closeFile(fPointer1, argv[optind]);
    closeFile(fPointer2, argv[optind + 1]);

    return EXIT_SUCCESS;
}


/**
 * @brief a function used to correctly close a file or throw an error if not
 *
 * @param file The file that needs to be closed
 * @param name The name of the file, that needs to be closed
 */
static void closeFile(FILE * fPointer, char* fName) {
    if(fclose(fPointer) != 0) {
        fprintf(stderr, "Error can't close %s file\n", fName);
        exit(EXIT_FAILURE);
    }
}


/**
 * @brief a function used to compare the two files and counts the number of characters
 * that differ fro one another each line. It either writes the result on a file, if one
 * is given or prints it out.
 *
 * @param cmp Integer value that lets us know, if the compare is case sensitive or not
 * @param fp1 The first input file, from which we read and compare the characters to
 * @param fp2 The second input file, from which we read and compare the characters to
 * @param fp3 The output file, in which we write the result, if the user has specified so
 */
static void calculateDifference(int cmp, FILE *fp1, FILE *fp2, FILE *fp3) {

    // Compare function, if -i is not defined then we use the function strncmp, otherwise we use strncasecmp
    int (*comp)(const char * s1, const char *s2, size_t n) = strncmp;

    if(cmp == 1){ // if cmp equals 1 means that -i was defined
        comp = strncasecmp;
    }

    int fChar1 = fgetc(fp1); // Getting first character of first file
    int fChar2 = fgetc(fp2); // Getting first character of second file
    int cnt; // Count for different characters in one line
    int i = 1; // Lines count
    char message[1024]; // Output message
    // int unevenLines = 0; // Kind of like a boolean, if its 0, the lines are even, if not they have different length

    while(fChar1 != EOF && fChar2 != EOF){
        cnt = 0;

        while(fChar1 != '\n' && fChar1 != EOF && fChar2 != '\n' && fChar2 != EOF){ // Iterating for each line

            if((*comp)((const char *) &fChar1, (const char *) &fChar2, 1) != 0){
                cnt++;
            }
            fChar1 = fgetc(fp1);
            fChar2 = fgetc(fp2);
        }
        // if the lines are uneven then we iterate till we reach the end of the line
        while(fChar1 != '\n' && fChar1 != EOF){
            fChar1 = fgetc(fp1);
        }
        while(fChar2 != '\n' && fChar2 != EOF){
            fChar2 = fgetc(fp2);
        }

        if(cnt > 0){
            snprintf(message, sizeof(message), "Line: %d, characters: %d\n", i, cnt);
            if(fp3 == NULL){
                printf("%s", message);
            } else {
                fputs(message, fp3);
            }
        }

        if(fChar1 == '\n' && fChar2 == '\n') { // if both char are \n then we move to a new line
            fChar1 = fgetc(fp1);
            fChar2 = fgetc(fp2);
            i++;
        }
    }
}
