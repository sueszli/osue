/**
 * @file io_driver.c
 * @author Lukas Thurner MatrNr.: 01427205 <lukas.thurner@tuwien.ac.at>
 * @date 2.11.2021
 * 
 * @brief Implementation of reading and writing 
 * 
 * Manage the reading and writing for the input and output files
 */  

#include <stdio.h>
#include "searcher.h"
#include <sys/errno.h>
#include <stdlib.h>
#include <stdbool.h>



/**
 * @brief Writes a line to the stdout
 */
void writeLineToTerminal(char* line){
	printf("%s\n",line);
}

/**
 * @brief Writes a line to the output file.
 */
void writeLineToFile(char* line, char* output_filename){
	
	FILE *file_pointer;
	
	//! checks if output file could be open	
	if( (file_pointer = fopen(output_filename, "a")) == NULL){
		fprintf(stderr, "[%s] ERROR: fopen failed: %s\n","mygrep",  strerror(errno));
		exit(EXIT_FAILURE); 
	}
	fprintf(file_pointer, "%s", line);
	fclose(file_pointer);
}

/**
 * @brief Open all input files and search for the keyword. If dif_up_low is true, the searcher differentiate between lower case and upper case letters 
 */ 

void searchInputFile(char* filename, char* keyword, char* output_filename, bool dif_up_low){

	FILE *file;
	
	//! checks if input file could be opened 
	if( (file = fopen(filename, "r")) == NULL){
		fprintf(stderr, "[%s] ERROR: fopen failed: %s\n","mygrep",  strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	//! search for the keyword line by line in the input files
	while((linelen = getline(&line, &linecap, file)) > 0){
		if(search_string(keyword, line, dif_up_low)){
			if(output_filename != NULL){
				printf("Write results to file: %s\n", output_filename);
				writeLineToFile(line, output_filename);
			}else{
				printf("Write results in terminal\n");
				writeLineToTerminal(line);
			}
		}
	}
	
	fclose(file);

}

/**
 * @brief Listen to the stdin and search for the keyword. If dif_up_low is true, the searcher differentiate between lower case and upper case letters
 */

void searchStdInput(char* keyword, bool dif_up_low){

    FILE *file = stdin;

    
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    //! search for the keyword in the line of the stdin
    while((linelen = getline(&line, &linecap, file)) > 0){

        if(search_string(keyword, line, dif_up_low)){
            writeLineToTerminal(line);
            
        }
    }
    
    fclose(file);

}



