/**
 * @file main.c
 * @author Lukas Thurner MatrNr.: 01427205 <lukas.thurner@tuwien.ac.at>
 * @date 1.11.2021
 *
 * @brief Main program module.
 * 
 * This program search for keyword in stdlin or inputfiles and writes the resul * ts in a file or stdout 
 **/

#include <stdlib.h>
#include <stdio.h>
#include "searcher.h"
#include "io_handler.h"
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>

/**
 * Program entry point.
 * 
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */

int main(int argc, char* argv[]){

	printf("mygrep is running ...\n");
		
	char *a_arg = NULL;
	int opt_o = 0;
	int c;

	char *keyword;
	bool dif_up_low = true;

	while( ( c = getopt(argc, argv, "o:i")) != -1) {
		switch(c){
			case 'i': opt_o++;
				dif_up_low = false;
				break;
			case 'o': a_arg = optarg;
				break;
			case '?':
				break;
            default:
                assert(0);
                break;
		}
	}
	
    // no arg entered in the commandline -> wrong input show description
    if(argv[optind] == NULL){
         fprintf(stderr, "mygrep [-i] [-o outputfile] keyword [file...]");
         exit(EXIT_FAILURE);
    }

	if(a_arg != NULL){
        printf("Write results in file: %s\n", a_arg);
      
	}
	
	keyword = argv[optind];
	
	
	if( (argc - optind) > 1){
		int input_files_counter = argc - (optind + 1);
		printf("Found %i input file(s)\n",input_files_counter);
		for(int i = (optind + 1); i < argc; i++){ 
			printf("Reading file: %s \n",argv[i]);
			searchInputFile(argv[i], keyword, a_arg, dif_up_low);
		}
		   
    }else{
        searchStdInput(keyword, dif_up_low);
    }
	

	return 0;
}
