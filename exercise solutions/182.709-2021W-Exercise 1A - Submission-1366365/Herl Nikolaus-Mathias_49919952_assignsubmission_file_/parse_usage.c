#include <stdio.h>
#include <unistd.h> // for getopt, ..?
#include <stdlib.h> // for memory allocation stuff like malloc, free, ...
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <limits.h>


// local
#include "parse_usage.h"
#include "memMngmt.h"


// usage see main.c

//local global variables
bool oFlag = false;

void checkAndSetFlag(bool *flag) {
    if (*flag == true)
        abortWUsageError();
    else
        *flag = true;
}

/*
Input input_init = {
        outFILE: NULL,
        inFILEs: NULL,
        nInFiles_P: 0
*/

Input parse_usage(int argc, char **argv) {
    Input config = {NULL, NULL, 0};

    int curFlag;
    //read t flag and possible output-file, catch incorrect program usage
    while ((curFlag = getopt(argc, argv, "o:")) != -1) {
        switch (curFlag) {
            case 'o':
                checkAndSetFlag(&oFlag);
                config.outFILE = fopen(optarg, "w");
                null_guard(config.outFILE, "fopen'ing outFILE failed");
                registerPtr(config.outFILE, Reg_fclose);
                break;
            case '?': /* invalid option */
                abortWUsageError();
                break;
            default:
                assert(false);
        }
    }

    // Default options
    if (!oFlag) {
        config.outFILE = stdout;
    }

    // Default options end

    // assert: optind == argv-index for the first non-option (-flag) argument
    if (argc == optind) {
        // means there was no non-option provided.
        config.n_infiles = 1;
        config.inFILEs = malloc(sizeof(FILE *));
        null_guard(config.inFILEs, "memory allocation for inFILE stdin failed");
        config.inFILEs[0] = stdin;
        registerPtr(config.inFILEs, Reg_free);
    } else {
        // populate inFILES_PPP
        config.n_infiles = argc - optind; // checked for correctness
        
        config.inFILEs = calloc(config.n_infiles, sizeof(FILE *));
        null_guard(config.inFILEs, "memory allocation for inFILEs failed");
        registerPtr(config.inFILEs, Reg_free);

        // read in and try to open all specified in-files
        for (int i = 0; optind < argc; ++i, ++optind) {
            (config.inFILEs)[i] = fopen(argv[optind], "r");
            null_guard((config.inFILEs)[i], "fopen'ing an infile failed");
            registerPtr((config.inFILEs)[i], Reg_fclose);
        }
    }
    return config;
}

