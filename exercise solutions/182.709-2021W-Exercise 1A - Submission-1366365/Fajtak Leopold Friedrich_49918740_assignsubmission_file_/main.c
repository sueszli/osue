/**
 * @file main.c
 * @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
 * @brief Main program module
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

char *myprog = NULL;  /*! Program name argv[0] */
int numInputs = 0;    /*! Number of input streams used */
FILE **inputs = NULL; /*! The input streams used */
FILE *output = NULL;  /*! The output stream used */
char **inputPaths = NULL; /*! Filesystem path variables of input documents */

/**
 * Mandatory usage function
 * @brief Writes usage information to stderr
 * 
 */
static void usage(void);

/**
 * @brief frees allocated memory and closes opened streams
 */
static void cleanup(int status); 

int main(int argc, char ** argv){
    myprog = argv[0];
    char *o_arg = NULL;
    int tabstop = 8;
    int c = 0;
    int numFiles = 0;
    
    //argparsing
    while ( (c = getopt(argc, argv, "t:o:")) != -1){
        switch (c) {
            case 't' : tabstop = strtol(optarg, NULL, 10);
                if (tabstop <= 0)
                    usage();
                break;
            case 'o' : o_arg = optarg;
                break;
            case '?':
                usage();
            default:
                assert(0); //should never occur
        }
    }
    if (optind <= argc){
        inputPaths = argv+optind;
        numFiles = argc-optind;
    }
    
    //open input streams
    if(numFiles == 0){
        if((inputs = malloc(sizeof(FILE*))) == NULL)
            cleanup(EXIT_FAILURE);
        inputs[0] = stdin;
        numInputs = 1;
    } else {
        if((inputs = malloc(numFiles*sizeof(FILE*))) == NULL)
            cleanup(EXIT_FAILURE);
        for(numInputs=0; numInputs<numFiles; numInputs++){
            if((inputs[numInputs] = openFile(inputPaths[numInputs], "r")) == NULL){
                fprintf(stderr, "%s: failed to open file: %s", myprog, inputPaths[numInputs]);
                cleanup(EXIT_FAILURE);
                }
            //printLinesContainingFrom(files[i], keyword, !opt_i, o_arg);
        }
    }

    //open output stream
    if(o_arg == NULL){
        output = stdout;
    } else {
        if ((output = openFile(o_arg, "w")) == NULL){
            fprintf(stderr, "%s: failed to open file: %s", myprog, o_arg);
            cleanup(EXIT_FAILURE);
        }
    }

    //process inputs
    for(int i=0; i<numInputs; i++){
        if(expand(inputs[i], output, tabstop)==-1){
            fprintf(stderr, "%s: failed to process inputs", myprog);
            cleanup(EXIT_FAILURE);
        }
    }

    cleanup(EXIT_SUCCESS);
}

static void usage(void){
    fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file ...]\n", myprog);
    cleanup(EXIT_FAILURE);
}

static void cleanup(int status){
    for(int i=0; i<numInputs; i++){
        if(closeFile(inputs[i])!=0)
            fprintf(stderr, "%s error with file %s", myprog, inputPaths[i]);
    }
    if(closeFile(output)!=0)
        fprintf(stderr, "%s error with output stream", myprog); 
    free(inputs);
    exit(status);
}
