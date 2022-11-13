/**
 *  Author: @author Ruben Patscheider
 *  MatrikelNummer: 01627951
 *  Date: @date 20.11.2020
 *  Name of project: mydiff
 *  Name of program: mydiff
 *  Name of module: mydiff
 *  Purpose of module: 
 *  @brief a porgramm that handles two files and outputs in which line and how many differences are in each line of the file
 *
 *  @details mydiff takes at least two input files as arguments reads as many lines as the shorter of the two files has and compares them.
 *  if a difference in the characters between the two lines is found, an output with line number and amount of different characters is generated.
 *  the user may specify if lower and upper case letters shall be seen as equal with -i, and if the output should be written into a file with -o
 *  followed by the name of the ouput file
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUF 256 ///< maximum number of bytes within a line can be read, can be made higher 

///< function to make all letters from the @param char array lowercase 
void toLowerCase(char* input){
    int i = 0;
    while(input[i] != '\n'){
        input[i] = tolower(input[i]);
        i++;
    }
}


int main(int argc, char* argv[]){


//Argument handling
    char *outfilename = NULL; ///< name of the outfile name, if give
    int i_count = 0, o_count = 0; ///< counter variables used in argument handling 
    int c; ///< temporary variable that saves each argument given by getopt to compare
    while((c = getopt(argc, argv, "o:i")) != -1){
        switch(c){
            case 'o':
                o_count++;
                outfilename = optarg;
                break;
            case 'i':
                i_count++;
                break;
            case '?':
                fprintf(stderr, "Error in %s: Unknown option. Valid options include -i and -o:\n", argv[0]);
                return EXIT_FAILURE;
            default:
                assert(1 != 1); ///< case can not be reached, assert is always false
        }
    }

///< Amount of input arguments is checked and errors are handlede accordingly
    int check_args = argc - (o_count + i_count + 1 * o_count);
    if(check_args < 3){
        fprintf(stderr, "Error in %s: Enter at least two filenames!\n", argv[0]);
        return EXIT_FAILURE;
    }
    if(argc > 6){
        fprintf(stderr, "Error in %s: Too many arguments! \n", argv[0]);
        return EXIT_FAILURE;
    }
    if(o_count > 1){
        fprintf(stderr, "Error in %s: Enter only one specified outputfile!\n", argv[0]);
        return EXIT_FAILURE;
    }

///< Opening of files and error andlings

    char *file1name = argv[argc-2]; ///< name of file one 
    char *file2name = argv[argc-1]; ///< name of file two

    FILE *file1 = fopen(file1name, "r"); ///< files are opened with read only. if the opening failed, and error is thrown
    if(file1 == NULL){
        fprintf(stderr, "Error in %s: Opening file 1 failed: %s\n", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    FILE *file2 = fopen(file2name, "r");
    if(file2 == NULL){
        fprintf(stderr, "Error in %s: Opening file 2 failed: %s\n", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    FILE *outfile = stdout; ///< if no output file is specifed by the user, the output is written to stdout
    if(o_count == 1){
        outfile = fopen(outfilename, "w"); ///< output file is opened as write
        if(outfile == NULL){ ///< if the opening failed and error is thrown
            fprintf(stderr, "Error in %s: Opening outfile failed: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }

///< computation of mydiff, 

    char bufffile1[MAX_BUF]; ///< temprary buffer for the inputline to be stored
    char bufffile2[MAX_BUF];
    int linecount = 1; ///< current line needed for output

    while(fgets(bufffile1, MAX_BUF, file1) != NULL && fgets(bufffile2, MAX_BUF, file2) != NULL){
        if(i_count == 1){ ///< if -i is specifed both strings are lowercase'd to be equal
            toLowerCase(bufffile1);
            toLowerCase(bufffile2);
        }
        int difcount = 0; ///< saving the amount of differences found
        int i = 0;
        while(bufffile1[i] != '\n' && bufffile2[i] != '\n'){
            if(bufffile1[i] != bufffile2[i]){
                difcount++;
            }
            i++;
        }
        if(difcount > 0){ ///< generate output for the user to read
            fprintf(outfile, "Line: %d, characters: %d\n", linecount, difcount);
        }
        linecount++;
    }



///< resource cleanup

    fclose(file1);
    fclose(file2);
    if(o_count == 1){
        fclose(outfile);
    }
    
    return EXIT_SUCCESS;
}