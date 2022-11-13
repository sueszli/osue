#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

/**
 * @file myexpand.c
 * @author Moritz Schalk (52005233)
 * @date 25 Oct 2021
 * @brief Read a textfile line by line and replace all tabs with spaces.
 * 
 * @details USAGE: myexpand [-t tabstop] [-o outfile] [file...]
 *
 *
 * The user can specify the number of spaces each tab should be replaced with usting the -t option.
 * -o selects a filename for the programs output.
 * If none is chosen, the programs output is written to stdout.
 *
 * Input files are given as positional arguments, if none are specified stdin is used as input.
 */

/**
 * @brief Print the usage message to the console.
 *
 * @param progname The programs name as seen by the user (i.e. argv[0]) 
 */
static void print_synopsis(char *progname);

/**
 * @brief Replace all tabs in a line with spaces.
 *
 * @param new_line       The pointer to the processed string null ('\0') terminated string is written here.
 * @param line           The input string. Its contents are copied to new_line with all tabs replaced by spaces.
 * @param line_len       Length of line.
 * @param spaces_per_tab Amount of spaces each tab should be replaced with.
 *
 * @detail It is the users responsibillity to free any memory that was allocated for new_line. 
 */
static void tabs_to_spaces(char **new_line, char *line, size_t line_len, int spaces_per_tab);

/**
 * @brief Replace tabs in @p input with a specified number of spaces and write them to @p output.
 *
 * @param input           The file handle that should be read from.
 * @param output          The file handle that should be written to.
 * @param spacces_per_tab The number of spaces that each tab is replaced with.
 */
static void process_file(FILE *input, FILE *output, int spaces_per_tab);

int main(int argc, char **argv) {
	// Parse command line options.
	int spaces_per_tab = -1;
	char *output_filename = NULL;
	char *input_filename  = NULL;
	
	char *optstring = "t:o:";	
	char opt = getopt(argc, argv, optstring);
	while (opt != -1) {
		char *conversion_error = NULL;
		switch (opt) {
			case 't':
				if (spaces_per_tab != -1) {
					fprintf(stderr, "%s: -t specified twice!\n", argv[0]);
					print_synopsis(argv[0]);
					exit(EXIT_FAILURE);	
				}

				spaces_per_tab = (int) strtol(optarg, &conversion_error, 10);
				if (strcmp(conversion_error, "\0") != 0 || spaces_per_tab < 0) {
					fprintf(stderr, "%s: Invalid argument %s! Must be a poitive integer!\n", argv[0], optarg);
					print_synopsis(argv[0]);	
					exit(EXIT_FAILURE);
				}
				break;
			
			case 'o':
				if (output_filename != NULL) {
					fprintf(stderr, "%s: -o specified twice!\n", argv[0]);
					print_synopsis(argv[0]);
					exit(EXIT_FAILURE);	
				}				
				output_filename = optarg;
				break;
			
			case '?':
				print_synopsis(argv[0]);
				exit(EXIT_FAILURE);
				break;
			
			default:
				//this should never be reached
				assert(0);
				break;	
		}
	 	opt = getopt(argc, argv, optstring);
	}
	
	
	//set default spaces per tab
	if (spaces_per_tab == -1) spaces_per_tab = 8;

	FILE *input_file = NULL;
	FILE *output_file = NULL;
	
	if (output_filename != NULL)	
		output_file = fopen(output_filename, "w");
	else
		output_file = stdout;

	if (output_file == NULL) {
		fprintf(stderr, "%s: Error opening output file: %s\n", argv[0], strerror(errno));
		exit(EXIT_FAILURE);	
	}
		
	if ((argc - optind) == 0) {
		// No input file specified, read from stdin instead.
		process_file(stdin, output_file, spaces_per_tab);
		fclose(output_file);
		exit(EXIT_SUCCESS);
	}
	
	while ((argc - optind) > 0) {
		input_filename = argv[optind];	
		
		input_file = fopen(input_filename, "r");
		
		if (input_file == NULL) {
			fprintf(stderr, "%s: Error opening input file \"%s\": %s\n", argv[0], input_filename, strerror(errno));
			fclose(output_file);
			exit(EXIT_FAILURE);	
		}
		
		process_file(input_file, output_file, spaces_per_tab);
		fclose(input_file);
		optind++;
	}
	
	fclose(output_file);
	
	return 0;
}

static void process_file(FILE *input_file, FILE *output_file, int spaces_per_tab) {
	char *line = NULL;
	size_t line_len = 0;

	while (getline(&line, &line_len, input_file) != -1) {
		char *new_line = NULL;
		tabs_to_spaces(&new_line, line, line_len, spaces_per_tab);
		
		fprintf(output_file, "%s", new_line);
		free(new_line);
	}

	free(line);
}

static void tabs_to_spaces(char **new_line, char *line, size_t line_len, int spaces_per_tab) {
	// Count the tabs within the given line and allocate an appropriate amount of memory.
	unsigned int tabs = 0;	
	unsigned int position = 0;
	
	while (line[position] != '\0') {
		if (line[position] == '\t')
			tabs++;

		position++;		
	}
	
	size_t new_len = line_len + (tabs * spaces_per_tab) + 1;
	char *buffer = (char*) malloc(new_len * sizeof(char));
	
	// Iterate over the input again, copying it character by character and writing the
	// right amount of spaces in place of each tab.
	
	position = 0;
	unsigned int new_line_position = 0;
	while(line[position] != '\0') {
		if (line[position] == '\t') {
			for (int i = 0; i < spaces_per_tab; i++) {
				buffer[new_line_position] = ' ';
				new_line_position++;
			}
		} else {
			buffer[new_line_position] = line[position];
			new_line_position++;
		}

		position++;
	}

	buffer[new_line_position] = '\0';

	*new_line = buffer;
}

static void print_synopsis(char *progname) {
	printf("Usage: %s [-t tabstop] [-o outfile] [file...]\n", progname);	
}
