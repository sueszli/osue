
/**
 * @file readArgs.c
 * @author Stefan Moser 12025955
 * @date 9.11.2021
 *
 * @brief argument handling
 *
 * @details the module provides a set of functions operating on the datastructure InputArguments_t
 **/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include "readArgs.h"

void initArguments(InputArguments_t *pA){
	pA->caseSensitive = 1;
	pA->pOutfileName = NULL;
	pA->outfile = NULL;
	pA->pKeyword = NULL;
	pA->pInputFiles = NULL;
	pA->countInputFiles = 0;
	pA->inputFiles = NULL;
}

void printInputArg(InputArguments_t *pA){
	if(pA->caseSensitive) printf("casesensitive\n");
	else printf("caseinsensitive\n");
	if(pA->pOutfileName) printf("Outfilename: %s\n",pA->pOutfileName);
	else printf("No Outfilename\n");
	if(pA->pKeyword) printf("Keyword: %s\n",pA->pKeyword);
	if(pA->countInputFiles <= 0) printf("No Inputfiles\n");
	else{
		int isOpen;
		for(int i=0; i < pA->countInputFiles && pA->pInputFiles != NULL; i++){
			if(pA->inputFiles && pA->inputFiles[i]) isOpen=1;
			else isOpen=0;
			printf("Inputfilename at %u: %s, isOpen=%i\n",i,pA->pInputFiles[i],isOpen);
		}
	}
	if(pA->outfile) printf("outfile is open\n");
}

void cleanUpInputArgs( InputArguments_t *pA){
	if(pA->outfile) {
		fclose(pA->outfile);
		pA->outfile = NULL;
	}
	if(pA->inputFiles){
		for(int i=0; i< pA->countInputFiles; i++) if(pA->inputFiles[i]) fclose(pA->inputFiles[i]);
		free(pA->inputFiles);
		pA->inputFiles = NULL;
	}
}

void cleanUpInputArgsAndExit(InputArguments_t *pA, int exitStatus){
	cleanUpInputArgs(pA);
	exit(exitStatus);
}


void analyzeInputArguments(int argc, char **argv, InputArguments_t *pA){
	int c;
	int keyWordPosition=1;
	/*get all options from argument line and determine resulting keyword position*/
	while ((c = getopt (argc, argv, "io:")) != -1)
	        switch (c)
	        {
	        case 'i':
	           pA->caseSensitive= 0;
	           keyWordPosition++;
	           break;
	        case 'o':
	           pA->pOutfileName = optarg;
	           keyWordPosition += 2;
	           break;
	        case '?':
	            if (optopt == 'o') fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        	    else if (isprint (optopt)) fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        	    else fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
                    cleanUpInputArgsAndExit(pA, EXIT_FAILURE);
	        default:
	           break;
	        }
	/*check if keyword exists*/
	if(keyWordPosition >= argc){
		fprintf(stderr,"[%s] ERROR: missing keyword\n",__FILE__);
	        cleanUpInputArgsAndExit(pA, EXIT_FAILURE);
	}
	else pA->pKeyword = argv[keyWordPosition];

	/*derive start and number of input files*/
	if(keyWordPosition+1 < argc){
		pA->pInputFiles = &argv[keyWordPosition+1];
		pA->countInputFiles =(argc-1)-keyWordPosition;
	}
	/*open file and allow writing*/
	if(pA->pOutfileName) {
		if( (pA->outfile = fopen(pA->pOutfileName,"w")) == NULL){
			fprintf(stderr, "fopen failed: %s. File not processed\n", strerror(errno));
			cleanUpInputArgsAndExit(pA, EXIT_FAILURE);
		}
	}

	if(pA->countInputFiles > 0) {
		int cInFiles = 0;
		/*allocate memory for all input files*/
		pA->inputFiles = (FILE**) calloc(pA->countInputFiles, sizeof(FILE *));
		if( pA->inputFiles == NULL ) {
			fprintf(stderr, "[%s] ERROR: memory allocation failed at line %u\n",__FILE__,__LINE__);
			cleanUpInputArgsAndExit(pA, EXIT_FAILURE);
		}

		/*open all input files for reading purposes*/
		for(int i = 0; i < pA->countInputFiles; i++){
			if( (pA->inputFiles[i] = fopen(pA->pInputFiles[i], "r") ) == NULL)
				fprintf(stderr, "fopen failed: %s. File not processed\n", strerror(errno));
			else cInFiles++;
		}

		/*check if all files opened correcty*/
		if(cInFiles != pA->countInputFiles) {
			fprintf(stderr, "ERROR: reading input files failed. Processing stopped!");
			cleanUpInputArgsAndExit(pA, EXIT_FAILURE);
		}
	}
}
