/**
 * @file main.c
 * @author Michael Zischg <Matriculation Number: 12024010>
 * @date 02.11.2021 
 * 
 * @brief Main program module
 * 
 * @details Reads input and compares both files. Outputs result to specified file or stdout. 
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include "mydiff.h"

static char *pgm_name; /**< The program name. */

void usage(void) {
    fprintf(stderr,"%s: Wrong input format. Two files are required at the end. '-o' requires one more file\n",pgm_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief The program starts here. All parameters to the function are checked and if valid 
 * given to function compareFiles in mydiff. Output is either written to file or or stdout. \n
 * global variables: pgm_name
 * @details (1) Option -i will ignore case sensitivity when comparing files. By default 
 * comparison is case sensitive. \n
 * (2) Option -o <filename> will write output to specified file. By default it will be written 
 * to stdout.  
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[]) {
    char *o_arg = NULL;
    int caseSensitive = 1;
    int c; 

    pgm_name = argv[0];

    while ( (c = getopt(argc, argv, "io:")) != -1) {
        switch(c) {
            case 'i': caseSensitive = 0;
                break;
            case 'o': o_arg = optarg;
                break;
            case '?':
                usage();
            default: break;
        }
    }

    if(argc != optind+2) {
        usage();
    }

    FILE *fp1 = fopen(argv[optind],"r");
    FILE *fp2 = fopen(argv[optind+1],"r");

    
    if(fp1 == NULL) {
        fprintf(stderr,"Error opening %s in %s: %s\n",argv[optind],pgm_name,strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(fp2 == NULL) {
        fprintf(stderr,"Error opening %s in %s: %s\n",argv[optind+1],pgm_name,strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(o_arg != NULL) {
        FILE *outputfile = fopen(o_arg,"w");
        compareFiles(fp1,fp2,caseSensitive,outputfile); 
        fclose(outputfile);
    } else {
        compareFiles(fp1,fp2,caseSensitive,stdout); 
    }
    
    if (fclose(fp1) != 0) {
        fprintf(stderr,"Error closing file %s in %s: %s\n", argv[optind],pgm_name,strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (fclose(fp2) != 0) {
        fprintf(stderr,"Error closing file %s in %s: %s.\n", argv[optind+1],pgm_name,strerror(errno));
        exit(EXIT_FAILURE);
    }
    fp1 = NULL;
    fp2 = NULL;

    exit(EXIT_SUCCESS);
}