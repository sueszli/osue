/**
 * @file mydiff.c
 * @author Filip Markovic, 12024750
 * @brief This program compares two files by reading them line by line, simple functions where used to accomplish this.
In every line the line with the fewer characters is compared to the other one. If they match until the full length of 
the whorter line it is a match, if not it's not. For every non matching line, it should be printed in what line
we currently are and how many characters differ.
 * @date 12.11.2021
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief This function implements the on the top given algorithm by using fopen() and various other
 * functions. The strings are saved into the buffer where they are converted and then compared,
 * if they are the same, nothing happens, otherwise the line and nuumber of wronk chars is saved.
 * 
 * @param read1 this is the path to the first file to compare, which compares to read 2
 * @param read2 second file to be read and compared with read1
 * @param caseinsens parameter to define if we should check for case insensivity
 * @param saveto path to save to (optional)
 */
void mydiff(char read1[], char read2[], int caseinsens,  char saveto[]);

/**
 * @brief main function to call, handles the given arguments and gives them further to mydiff()
 * 
 * @param argc number of givent arguments
 * @param argv array of given arguments
 * @return int 0 if program worked, 1 if error
 */

int main(int argc, char *argv[]){
    int c;
    int opt_i = 0;
    int opt_o = 0;
    char *o_arg = NULL;
    char *file1, *file2;

    // check for optional arguments
    while ( (c = getopt(argc, argv, "io:")) != -1){
        switch (c) {
            case 'i' : opt_i += 1;
                break;
            case 'o' : o_arg = optarg;
                opt_o += 1;
                break;
            // case '?' : fprintf(stderr, "[%s] ERROR: wrong number of positional arguments \n SYNOPSIS mydiff [-i] [-o outfile] file1 file2\n", argv[0]);
            //     exit(1);
            //     break;
            // getopt() handles the case of an input being invalid
        }
    }

    // too many/few optional argumetns, program exits with error
    if ((opt_i != 0 && opt_i != 1) || (opt_o != 0 && opt_o != 1)){
        printf("opt_i %i\n", opt_i);
        printf("opt_o %i\n", opt_o);
        fprintf(stderr, "[%s] Error: wrong number of optional arguments \n SYNOPSIS mydiff [-i] [-o outfile] file1 file2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // if the positional arguments don't equal two, the input is wrong and the program exits
    if ( (argc - optind) != 2 ) {
        fprintf(stderr, "[%s] Error: wrong number of positional arguments \n SYNOPSIS mydiff [-i] [-o outfile] file1 file2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // save the paths
    file1 = argv[optind];
    file2 = argv[optind + 1];

    // run function on files
    mydiff(file1, file2, opt_i, o_arg);
    exit(EXIT_SUCCESS);
}

void mydiff(char read1[], char read2[], int caseinsens, char saveto[]){

    // initialize needed variables

    FILE *file1 = fopen(read1, "r");
    FILE *file2 = fopen(read2, "r");
    char buffer1[1024];
    char buffer2[1024];
    char line1[20], line2[20];
    int line = 1;
    char help[1024];

    // fopen failed, program error
    if ( file1 == NULL || file2 == NULL ){
        fprintf(stderr, "Error: fopen failed %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // path given
    if (saveto != NULL){
        // open file in which we save
        FILE *writeto = fopen(saveto, "w");

        // read line by line of file (untile there are no lines left)
        while (fgets(buffer1, sizeof(buffer1), file1) != NULL && fgets(buffer2, sizeof(buffer2), file2) != NULL){

            // caseinsens == 1, if caseinsensivity is turned on
            if (caseinsens == 1){
                for (int i = 0; i < strlen(buffer1); i++)
                {
                    buffer1[i] = tolower(buffer1[i]);
                    buffer2[i] = tolower(buffer2[i]);
                }
            }

            // save only string part of buffer (without this buffer has an extra char in strlen())
            sscanf(buffer1, "%s", line1);
            sscanf(buffer2, "%s", line2);

            // lines differ -> look for differing characters
            if (strncmp(line1, line2, (strlen(line1) < strlen(line2) ? strlen(line1) : strlen(line2))) != 0){
                // counts differing characters 
                int characters = 0;
                // while loop variable
                int i = 0;
                // go through string and check for differences
                while (i < (strlen(line1) < strlen(line2) ? strlen(line1) : strlen(line2))){
                    if (buffer1[i] != buffer2[i]){
                        characters++;
                    }
                    i++;
                }
                // save differing line and characters to writeto
                // have to convert to a string first with sprintf
                sprintf(help, "Line: %i, characters: %i \n", line, characters);
                if (fputs(help, writeto) == EOF){
                    fprintf(stderr, "Error: fputs failed %s\n", strerror(errno));
                }
            } 
            line++;
        }
    } 
    // no path given
    else {
        // read line by line of file (untile there are no lines left)
        while (fgets(buffer1, sizeof(buffer1), file1) != NULL && fgets(buffer2, sizeof(buffer2), file2) != NULL){

            // caseinsens == 1, if caseinsensivity is turned on
            if (caseinsens == 1){
                for (int i = 0; i < strlen(buffer1); i++)
                {
                    buffer1[i] = tolower(buffer1[i]);
                    buffer2[i] = tolower(buffer2[i]);
                }
            }

            // save only string part of buffer (without this buffer has an extra char in strlen())
            sscanf(buffer1, "%s", line1);
            sscanf(buffer2, "%s", line2);

            // lines differ -> look for differing characters
            if (strncmp(line1, line2, (strlen(line1) < strlen(line2) ? strlen(line1) : strlen(line2))) != 0){\
                // counts differing characters 
                char characters = 0;
                // while loop variable
                int i = 0;
                // go through string and check for differences
                while (i < (strlen(line1) < strlen(line2) ? strlen(line1) : strlen(line2))){
                    if (buffer1[i] != buffer2[i]){
                        characters++;
                    }
                    i++;
                }
                // save differing line and characters to writeto
                // have to convert to a string first with sprintf
                printf("Line: %d, characters: %d \n", line, characters);
            } 
            line++;
        }
    }
    // close opened files and exit program
    fclose(file1);
    fclose(file2);
    exit(EXIT_SUCCESS);
}