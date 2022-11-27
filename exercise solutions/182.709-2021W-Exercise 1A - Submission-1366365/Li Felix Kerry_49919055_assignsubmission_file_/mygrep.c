/**
 * @file mygrep.c
 * @author Felix Kerry Li <e01634055@student.tuwien.ac.at>
 * @date 05.11.2021
 *
 * @brief mygrep for 1A
 *
 * This program implements the mygrep function for 1A
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>

/**
 *
 * @brief This function prints each line from the given inputstream to the given outputstream
 *
 * @param keyword the keyword each line should contain
 * @param i_arg the flag if the program should differentiate between lower and upper cases
 * @param inputstream the inputstream to read
 * @param outputstream the outputstream to write
 **/
static void printlines(char *keyword, int i_arg, FILE *inputstream, FILE *outputstream) {

	char *line = NULL;
	size_t laenge = 0;
	ssize_t nread;

	char grep_keyword[strlen(keyword)];

	for(int i=0; keyword[i]; i++) {
		if(i_arg) {
			grep_keyword[i] = tolower(keyword[i]);
		} else {
			grep_keyword[i] = keyword[i];
		}
	}

	while((nread = getline(&line, &laenge, inputstream)) != -1) {
				
		char grep_line[laenge];

		for(int i=0; line[i]; i++) {
			if(i_arg) {
				grep_line[i] = tolower(line[i]);
			} else {
				grep_line[i] = line[i];
			}
		}		

		if(strstr(grep_line, grep_keyword) != NULL) {
			fprintf(outputstream, "%s", line);
		}		
	}

	free(line);
}

/**
 *
 * @brief The program parses the arguments and checks for input and output files.
 *
 * @param argc argument counter
 * @param argv argument vector
 * @return EXIT_SUCCESS or EXIT_FAILURE
 **/
int main(int argc, char *argv[]) {

	int i_arg = 0;
	
	int o_arg = 0;
	
	int c;

	FILE *inputstream = stdin;

	FILE *outputstream = stdout;

	char *ovalue = NULL;
	
	while (( c = getopt(argc, argv, "io:")) != -1) {
		switch(c) {

			case 'i': i_arg = 1;
			break;

			case 'o': o_arg = 1;
			ovalue = optarg;
			break;
			
			case '?':
				if(optopt == 'c') {
				
				} else {
				
				}
			return 1;
			
			default: abort();
		}
	}

	char *keyword = argv[optind];

	if (argc < 2 || keyword == NULL) {
		//invalid number of arguments
		fprintf(stderr, "%s\n", "Invalid number of arguments!");
		exit(EXIT_FAILURE);

	} else {

		//check if outputfile is given
		if(o_arg) {
			outputstream = fopen(ovalue, "w");
			if(outputstream == NULL) {
				fprintf(stderr, "Error during opening outputstream!");
				exit(EXIT_FAILURE);
			}
		}

		//check if input files are not given
		if (optind == (argc - 1)) {
			//no input files given		
			printlines(keyword, i_arg, inputstream, outputstream);

		} else {
			//input files should start at optind

			for(int i = optind + 1; i < argc; i++) {
				inputstream = fopen(argv[i], "r");
				if(inputstream == NULL) {
					fprintf(stderr, "Error during opening inputstream!");
					exit(EXIT_FAILURE);
				}

				printlines(keyword, i_arg, inputstream, outputstream);

				fclose(inputstream);

			}

		}

		if(o_arg) {
			fclose(outputstream);
		}
	}

	return 0;
}