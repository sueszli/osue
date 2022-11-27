/**
 * @file mygrep.c
 * @author András Fekete 11776844
 * @date 9.11.2021
 * @brief Solution of exercise 1A
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

static char *myprog; // name of the program, used in error messages

/**
 * @brief prints out the correct usage of the program (synopsis)
 */
static void usage(void) {
	fprintf(stderr, "Usage %s [-i] [-o outfile] keyword [file...]", myprog);
	exit(EXIT_FAILURE);
}

/**
 * @brief parses an input file line by line, and prints the line out, if it contains keyword
 * @param input file to be read from
 * @param output file to be written to
 * @param case_sensitivity true if the comparison should be case sensitive, false otherwise
 * @param keyword the keyword to be searched for
 */
static void mygrep(FILE *input, FILE *output, bool case_sensitivity, char *keyword) {
	size_t len = 0;
	char *line = NULL;
	ssize_t read;
	char *line_copy;
	char *keyword_copy; 
	
	// parse file line by line
	while((read = getline(&line, &len, input)) != -1) {
		if(!case_sensitivity) {
			//make copies of line and keyword
			line_copy = strdup(line);

			if(line_copy == NULL) {
				free(line_copy);
				free(line);
				fprintf(stderr, "strdup failed %s\n", strerror(errno));
				exit(EXIT_FAILURE);	
			}

			keyword_copy = strdup(keyword);

			if(keyword_copy == NULL) {
				free(keyword_copy);
				free(line_copy);
				free(line);
				fprintf(stderr, "strdup failed %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			
			// transform copies to lowercase
			for(size_t i = 0; line[i]; i++) {
				line_copy[i] = tolower(line[i]);		
			}
	
			for(size_t i = 0; keyword[i]; i++) {
				keyword_copy[i] = tolower(keyword[i]);		
			}	
	
			if(strstr(line_copy, keyword_copy) != NULL) {
				if(fputs(line, output) == EOF) {
					free(line_copy);
					free(keyword_copy);
					free(line);
					fprintf(stderr, "fputs failed %s\n", strerror(errno));
					exit(EXIT_FAILURE);	
				}
			fflush(output);	
			}
			free(line_copy);
			free(keyword_copy);

		} else {
			if(strstr(line, keyword) != NULL) {
				if(fputs(line, output) == EOF) {
					free(line);
					fprintf(stderr, "fputs failed %s\n", strerror(errno));
					exit(EXIT_FAILURE);	
				}
			fflush(output);	
			}
		}
	}
	free(line);
}


/*
 * @brief parses arguments, sets up output file, and calls mygrep
 * @param argc argument counter
 * @param argv argument vector
 */
int main(int argc, char **argv) {
	myprog = argv[0];
	bool case_sensitivity = true;
	char *output_file = NULL;
	int opt_i = 0;
	int opt_o = 0;
	FILE *input;
	FILE *output = stdout;	
	char *keyword;
	int c;

	while ((c = getopt(argc, argv, "io:")) != -1) {
		switch(c) {
			case 'i':
				opt_i++;
				case_sensitivity = false;
				break;
			case 'o':
				opt_o++;
				output_file = optarg;
				break;
			case '?': usage();
				break;
			default: usage();
				break;
		}
	}
	
	if (opt_i > 1 || opt_o > 1 || argc == optind) {
		usage();
	}
	
	keyword = argv[optind];

	if (output_file != NULL) {
		output = fopen(output_file, "w");
		if (output == NULL) {
			fprintf(stderr, "fopen failed %s\n", strerror(errno));
			exit(EXIT_FAILURE);	
		}
	}
	
		
	if((argc - optind) == 1) {
		mygrep(stdin, output, case_sensitivity, keyword);
	} else {
		for(size_t i = optind + 1; i < argc; i++) {
			input = fopen(argv[i], "r");
			if(input == NULL) {
				fprintf(stderr, "fopen failed %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			mygrep(input, output, case_sensitivity, keyword);
			if(fclose(input) == EOF) {
				fprintf(stderr, "fclose failed %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}	
		}
	}

	if(fclose(output) == EOF) {
		fprintf(stderr, "fclose failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}	
	
	return 0;
}
